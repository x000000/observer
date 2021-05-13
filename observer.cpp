#include <chrono>
#include <util/dstr.h>
#include <util/platform.h>
#include <obs-module.h>
#include <obs-frontend-api.h>
#include "observer.hpp"
#include "observer-settings.hpp"
#include "ws-chat-client.hpp"

#define blog(log_level, format, ...)                    \
	blog(log_level, "[observer] " format, \
	     /*obs_source_get_name(context->source), */##__VA_ARGS__)

#define debug(format, ...) blog(LOG_DEBUG, format, ##__VA_ARGS__)
#define info(format, ...) blog(LOG_INFO, format, ##__VA_ARGS__)
#define warn(format, ...) blog(LOG_WARNING, format, ##__VA_ARGS__)

#define CONFIG "config.json"
#define JSON_SELF ""
#define MOD_NAME "observer"
#define MOD_SETTINGS_CHANNEL "channel"
#define MOD_SETTINGS_ACTIONS "actions"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(MOD_NAME, "en-US")

struct sceneitem_lookup_context {
    obs_sceneitem_t* pivot_item = nullptr;
    std::vector<obs_sceneitem_t*> items = std::vector<obs_sceneitem_t*>();
    bool is_scene_active = false;
};

bool scene_item_lookup(obs_scene_t *, obs_sceneitem_t *item, void *ctx)
{
    auto context = (sceneitem_lookup_context*) ctx;

    obs_source_t *source = obs_sceneitem_get_source(item);
    const char *name = obs_source_get_name(source);

    warn("Scene item: %s", name);

    if (strcmp(name, "Image B") == 0) {
        context->items.push_back(item);
        if (context->is_scene_active) {
            context->pivot_item = item;
        }
    }

    return true;
}

bool scene_lookup(void *ctx, obs_source_t *source)
{
    auto context = (sceneitem_lookup_context*) ctx;
    context->is_scene_active = obs_source_active(source);

    obs_scene_t *scene = obs_scene_from_source(source);
    obs_scene_enum_items(scene, scene_item_lookup, ctx);

    return true;
}

class ActionHandler
{
public:
    explicit ActionHandler(const action_descriptor &descr) : _pattern(QString::fromStdString(descr.expression))
    {
        for (const auto &user : descr.users) {
            if (user == "@mod") {
                _userType |= UserType::Mod;
            } else if (user == "@sub") {
                _userType |= UserType::Sub;
            } else if (user == "@vip") {
                _userType |= UserType::Vip;
            } else if (user == "@stuff") {
                _userType |= UserType::Stuff;
            } else {
                _users.push_back(QString::fromStdString(user));
            }
        }
        _anyUser = !_userType && _users.isEmpty();
        _actionType = descr.type;
        _timeout = descr.timeout;
    }

    void handle(const QString &user, const QString &message, const UserType &userFlags = UserType::None)
    {
        if (_anyUser || (_userType && (_userType & userFlags)) || (_users.contains(user))) {
            if (_pattern.indexIn(message) != -1) {
                exec();
            }
        }
    }
private:
    QRegExp _pattern;
    UserType _userType = UserType::None;
    ActionType _actionType;
    QVector<QString> _users;
    bool _anyUser;
    int _timeout;
    long long _lastExec = 0;

    inline void exec()
    {
        if (_timeout > 0) {
            auto time = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

            if (time - _lastExec < _timeout) {
                return;
            }

            _lastExec = time;
        }

        sceneitem_lookup_context ctx;
        obs_enum_scenes(scene_lookup, &ctx);

        bool visible;

        switch (_actionType) {
            case ActionType::Toggle:
                visible = ctx.pivot_item == nullptr || !obs_sceneitem_visible(ctx.pivot_item);
                break;
            case ActionType::Show:
                visible = true;
                break;
            case ActionType::Hide:
                visible = false;
                break;
            default:
                return;
        }

        for (auto it = ctx.items.begin(); it < ctx.items.end(); it++) {
            obs_sceneitem_set_visible(*it, visible);
        }
    }
};

action_descriptor::~action_descriptor()
{
    if (handler) {
        delete handler;
        handler = nullptr;
    }
}

static ObserverSettings *settings_window = nullptr;
static WsChatClient *client = nullptr;
static observer_settings mod_settings;

void on_priv_message_received(const QString &user, const QString &message, const UserType &flags)
{
    for (auto &h : mod_settings.actions) {
        if (h.handler) {
            h.handler->handle(user, message, flags);
        }
    }
}

#define obs_set(type, prop)  obs_data_set_##type(item, #prop, d.prop)
#define obs_set_string(prop) obs_data_set_string(item, #prop, d.prop.c_str())
#define obs_set_string_array(prop) {                           \
    auto _array = obs_data_array_create();                     \
    for (auto v : d.prop) {                                    \
        auto _item = obs_data_create();                        \
        obs_data_set_string(_item, JSON_SELF, v.c_str());      \
        obs_data_array_push_back(_array, _item);               \
        obs_data_release(_item);                               \
    }                                                          \
    obs_data_set_array(item, #prop, _array);                   \
}

static std::vector<std::string> read_string_array(obs_data_t *data, const char *prop)
{
    auto array = obs_data_get_array(data, prop);
    size_t count = obs_data_array_count(array);

    std::vector<std::string> vec;
    vec.reserve(count);

    for (size_t i = 0; i < count; i++) {
        auto item = obs_data_array_item(array, i);
        vec.push_back(obs_data_get_string(item, JSON_SELF));
        obs_data_release(item);
    }
    obs_data_array_release(array);

    return vec;
}

struct observer_settings read_settings()
{
    auto cfg_path = obs_module_config_path(CONFIG);
    auto settings = obs_data_create_from_json_file(cfg_path);
    bfree(cfg_path);

    observer_settings config;

    if (!settings) {
        return config;
    }

    config.channel = obs_data_get_string(settings, MOD_SETTINGS_CHANNEL);

    auto actions = obs_data_get_array(settings, MOD_SETTINGS_ACTIONS);
    size_t count = obs_data_array_count(actions);

    for (size_t i = 0; i < count; i++) {
        auto item = obs_data_array_item(actions, i);

        config.actions.push_back({
            .type       = static_cast<ActionType>(obs_data_get_int(item, "type")),
            .sceneitems = read_string_array(item,   "sceneitems"),
            .users      = read_string_array(item,   "users"),
            .expression = obs_data_get_string(item, "expression"),
            .is_active  = obs_data_get_bool(item,   "is_active"),
            .timeout    = static_cast<int>(obs_data_get_int(item, "timeout")),
        });

        obs_data_release(item);
    }

    obs_data_array_release(actions);
    obs_data_release(settings);

    return config;
}

void save_settings(const observer_settings *config)
{
    auto array = obs_data_array_create();
    for (const action_descriptor& d : config->actions) {
        auto item = obs_data_create();

        obs_set_string_array(users);
        obs_set_string_array(sceneitems);

        obs_set_string(expression);
        obs_set(bool,  is_active);
        obs_set(int,   timeout);
        obs_set(int,   type);

        obs_data_array_push_back(array, item);
        obs_data_release(item);
    }

    auto settings = obs_data_create();
    obs_data_set_array(settings,  MOD_SETTINGS_ACTIONS, array);
    obs_data_set_string(settings, MOD_SETTINGS_CHANNEL, config->channel.c_str());
    obs_data_array_release(array);

    auto cfg_path = obs_module_config_path(CONFIG);
    obs_data_save_json(settings, cfg_path);
    bfree(cfg_path);

    obs_data_release(settings);
}

void init()
{
    mod_settings = read_settings();

    for (auto &d : mod_settings.actions) {
        if (d.is_active) {
            try {
                d.handler = new ActionHandler(d);
            } catch (...) {
                continue;
            }
        }
    }

    if (!mod_settings.channel.empty()) {
        client->join(mod_settings.channel);
    }
}

void on_setting_closed(bool result)
{
    if (result) {
        save_settings(settings_window->model()->settings());
        init();
    }
    delete settings_window;
    settings_window = nullptr;
}

bool obs_module_load(void)
{
    auto dir = obs_module_config_path(nullptr);
    if (dir) {
        os_mkdirs(dir);
        bfree(dir);
    }

    obs_frontend_add_tools_menu_item(obs_module_text("observer_settings.menuitem"), [](void *) {
        obs_frontend_push_ui_translation(obs_module_get_string);

        if (!settings_window) {
            settings_window = new ObserverSettings(read_settings());
            QObject::connect(settings_window, &ObserverSettings::closed, &on_setting_closed);
            settings_window->show();
        } else {
            settings_window->show();
            settings_window->raise();
        }

        obs_frontend_pop_ui_translation();
    }, nullptr);

//    signal_handler_t *handler = obs_get_signal_handler();
//    signal_handler_connect(handler, source_activate, on_signal, &source_activate);
//    signal_handler_connect(handler, source_show, on_signal, &source_show);

    return true;
}

void obs_module_post_load(void)
{
    client = new WsChatClient();
    init();
    QObject::connect(client, &WsChatClient::privMessageReceived, on_priv_message_received);
}

const char *obs_module_name(void)
{
    return MOD_NAME;
}