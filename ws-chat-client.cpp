#include "observer.hpp"
#include "ws-chat-client.hpp"
#include <util/dstr.h>
#include <random>
#include <set>

#define blog(log_level, format, ...) blog(log_level, "[observer] " format, ##__VA_ARGS__)

#define debug(format, ...) blog(LOG_DEBUG, format, ##__VA_ARGS__)
#define info(format, ...) blog(LOG_INFO, format, ##__VA_ARGS__)
#define warn(format, ...) blog(LOG_WARNING, format, ##__VA_ARGS__)

static std::random_device rd;  // only used once to initialise (seed) engine
static std::mt19937 rng(rd()); // random-number engine used (Mersenne-Twister in this case)
static std::uniform_int_distribution<int> user_random(10000, 99999);

static const QUrl wss_endpoint = QUrl(QStringLiteral("wss://irc-ws.chat.twitch.tv"));

static const char* cap_msg  = "CAP REQ :twitch.tv/tags twitch.tv/commands";
static const char* pass_msg = "PASS SCHMOOPIIE";

static const char* nick_msg = "NICK %s";
static const char* user_msg = "USER %s 8 * :%s";
static const char* join_msg = "JOIN #%s";
static const char* part_msg = "PART #%s";


WsChatClient::WsChatClient(QObject *parent) :
    QObject(parent)
{
    connect(&_socket, &QWebSocket::connected, this, &WsChatClient::onConnected);
    connect(&_socket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors), this, &WsChatClient::onSslErrors);
}

void WsChatClient::join(std::string room)
{
    if (_socket.isValid() && _room == room) {
        return;
    }

    auto old_room = _room;
    _room = QString::fromStdString(room).toLower().toStdString();
    _privmsg_marker = QString::fromStdString(" PRIVMSG #" + _room);

    if (_socket.isValid()) {
        send(QString::asprintf(part_msg, old_room.c_str()));
        send(QString::asprintf(join_msg, _room.c_str()));
    } else {
        _user = "justinfan" + std::to_string(user_random(rng));
        _welcome_msg = ":tmi.twitch.tv 001 " + _user + " :Welcome, GLHF!";
        _socket.open(QUrl(wss_endpoint));
    }
}

void WsChatClient::onConnected()
{
    connect(&_socket, &QWebSocket::textMessageReceived, this, &WsChatClient::onWelcomeReceived);

    send(cap_msg);
    send(pass_msg);
    send(QString::asprintf(nick_msg, _user.c_str()));
    send(QString::asprintf(user_msg, _user.c_str(), _user.c_str()));
}

void WsChatClient::onWelcomeReceived(QString message)
{
    debug(">> '%s'", message.toStdString().c_str());

    if (message.startsWith(_welcome_msg.c_str())) {
        disconnect(&_socket, &QWebSocket::textMessageReceived, this, &WsChatClient::onWelcomeReceived);
        connect   (&_socket, &QWebSocket::textMessageReceived, this, &WsChatClient::onTextMessageReceived);

        send(QString::asprintf(join_msg, _room.c_str()));
    }
}

void WsChatClient::onTextMessageReceived(QString message)
{
    QT_TRY {
        if (message.startsWith("PING ")) {
            debug(">> '%s'", message.toStdString().c_str());
            send("PONG");
        } else {
            QString name, text;
            UserType flags;
            if (tryParsePrivmsg(message, name, text, flags)) {
                debug(">> %s: %s", name.toStdString().c_str(), text.toStdString().c_str());
                emit privMessageReceived(name, text, flags);
            } else {
                debug(">> '%s'", message.toStdString().c_str());
            }
        }
    } QT_CATCH (...) {
        warn("Failed to handle incoming message");
    }
}

//#define DEBUG_PRIVMSG
#ifdef DEBUG_PRIVMSG
static std::set<std::string> ignore_props {
    "color",
    "display-name",
    "emotes",
    "id",
    "room-id",
    "tmi-sent-ts",
    "turbo",
    "user-id",
};
#endif

bool WsChatClient::tryParsePrivmsg(QString &message, QString &name, QString &text, UserType &flags)
{
    int pos = 0;
    int lastPos = 0;
    while (true) {
        pos = message.indexOf(" :", lastPos);
        if (pos == -1) {
            return false;
        }

        auto ref = message.midRef(lastPos, pos - lastPos);
        if (ref.endsWith(_privmsg_marker)) {
#ifdef DEBUG_PRIVMSG
            std::map<std::string, std::string> props;
#endif

            flags = UserType::None;
            for (auto &token : message.midRef(0, lastPos - 2).split(";")) {
                auto pair = token.split('=');
                auto prop = pair.first().toString().toStdString();

                if (prop == "subscriber") {
                    if ("1" == pair.last()) {
                        flags |= UserType::Sub;
                    }
                } else if (prop == "mod") {
                    if ("1" == pair.last()) {
                        flags |= UserType::Mod;
                    }
                } else if (prop == "badges") {
                    for (auto &b : pair.last().split(',')) {
                        if (b.startsWith("vip/")) {
                            flags |= UserType::Vip;
                        } else if (b.startsWith("stuff/")) {
                            flags |= UserType::Stuff;
                        }
                    }
                }
#ifdef DEBUG_PRIVMSG
                else if (!ignore_props.contains(prop)) {
                    props[prop] = pair.last().toString().toStdString();
                }
#endif
            }

            lastPos = pos + 2;
            pos = ref.indexOf("!");
            if (pos != -1) {
                name = ref.left(pos).toString();
                text = message.midRef(lastPos).trimmed().toString();
                return true;
            }
            break;
        }

        pos += 2;
        lastPos = pos;
    }

    return false;
}

void WsChatClient::send(const char* msg)
{
    debug("<< %s", msg);
    _socket.sendTextMessage(QString::fromUtf8(msg));
}

void WsChatClient::send(const QString msg)
{
    debug("<< %s", msg.toStdString().c_str());
    _socket.sendTextMessage(msg);
}

void WsChatClient::onSslErrors(const QList<QSslError> &errors)
{
    Q_UNUSED(errors);
    debug("-> SSL error received");
    // WARNING: Never ignore SSL errors in production code.
    // _socket.ignoreSslErrors();
}