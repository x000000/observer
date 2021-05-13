#pragma once

#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QSslError>

QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WsChatClient : public QObject
{
    Q_OBJECT
public:
    explicit WsChatClient(QObject *parent = nullptr);
    void join(std::string room);

Q_SIGNALS:
    void privMessageReceived(const QString &user, const QString &message, UserType &flags);

private Q_SLOTS:
    void onConnected();
    void onWelcomeReceived(QString message);
    void onTextMessageReceived(QString message);
    void onSslErrors(const QList<QSslError> &errors);


private:
    QWebSocket _socket;
    std::string _room;
    std::string _user;
    std::string _welcome_msg;
    QString _privmsg_marker;
    void send(const char* msg);
    void send(QString msg);
    bool tryParsePrivmsg(QString &message, QString &name, QString &text, UserType &flags);
};
