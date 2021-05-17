#include <set>
#include <obs-module.h>
#include "action-config-dialog.hpp"
#include "ui_action-config-dialog.h"

#define add_action(item) ui->actionType->addItem(obs_module_text(#item), item)

static const QRegExp line_split_regex(R"([\r\n]+)", Qt::CaseSensitive, QRegExp::RegExp2);

static QString implode(const std::vector<std::string> &vec, const char *delim = "\n")
{
    QString str;
    for (auto it = vec.begin(); it < vec.end(); it++) {
        str.append(it->c_str()).append(delim);
    }
    if (!str.isEmpty()) {
        auto len = strlen(delim);
        str.remove(str.length() - len, len);
    }
    return str;
}

static std::vector<std::string> to_std_vector(const QStringList &vec)
{
    std::vector<std::string> vector;
    vector.reserve(vec.size());

    for (auto it = vec.begin(); it < vec.end(); it++) {
        vector.push_back(it->toStdString());
    }
    return vector;
}

bool collect_sources(void *ctx, obs_source_t *source)
{
    obs_scene_t *scene = obs_scene_from_source(source);
    obs_scene_enum_items(scene, [](obs_scene_t *, obs_sceneitem_t *item, void *ctx) {
        const char *name = obs_source_get_name(obs_sceneitem_get_source(item));
        ((std::set<std::string> *) ctx)->insert(name);

        return true;
    }, ctx);

    return true;
}

QStringList get_source_items()
{
    std::set<std::string> ctx;
    obs_enum_scenes(collect_sources, &ctx);

    QStringList sources;
    sources.reserve(ctx.size());
    for (auto name : ctx) {
        sources.push_back(QString::fromStdString(name));
    }
    sources.sort();

    return sources;
}


ActionConfigDialog::ActionConfigDialog(action_descriptor *settings, QWidget *parent)
    : QDialog(parent), ui(new Ui_ActionConfigDialog)
{
    ui->setupUi(this);

    add_action(ActionType::Toggle);
    add_action(ActionType::Hide);
    add_action(ActionType::Show);

    ui->availableSources->addItems(get_source_items());
    ui->availableSources->setCurrentIndex(-1);

    auto index = ui->actionType->findData(settings->type);
    if (ui->actionType->currentIndex() != index) {
        ui->actionType->setCurrentIndex(index);
    }

    ui->users->setPlainText(implode(settings->users));
    ui->affectedSources->setPlainText(implode(settings->sceneitems));

    ui->pattern->setText(QString::fromStdString(settings->expression));
    ui->ignoreCase->setChecked(settings->ignore_case);
    ui->timeout->setValue(settings->timeout);

    QObject::connect(ui->availableSources, &QComboBox::currentTextChanged, [this](const QString &str) {
        if (!str.isEmpty()) {
            auto text = ui->affectedSources->toPlainText().append('\n').append(str);
            ui->affectedSources->setPlainText(text);
            ui->availableSources->setCurrentIndex(-1);
        }
    });
}

ActionConfigDialog::~ActionConfigDialog()
{
    delete ui;
}

void ActionConfigDialog::save(action_descriptor *settings)
{
    settings->type        = static_cast<ActionType>(ui->actionType->currentData().toInt());
    settings->sceneitems  = to_std_vector(ui->affectedSources->toPlainText().split(line_split_regex, Qt::SkipEmptyParts));
    settings->users       = to_std_vector(ui->users->toPlainText().split(line_split_regex, Qt::SkipEmptyParts));
    settings->expression  = ui->pattern->text().trimmed().toStdString();
    settings->ignore_case = ui->ignoreCase->isChecked();
    settings->timeout     = ui->timeout->value();
}
