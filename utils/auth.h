#ifndef AUTH_H
#define AUTH_H

#include <QString>
#include <QStringList>
#include <QNetworkCookie>
#include <QHttpServerRequest>
#include <QCoroTask>
#include <QCoroCore>
#include "utils/config.h"
#include "utils/qtbcrypt.h"
#include "httpHelper.h"

typedef struct{
    QString id;
    QString username;
    QDateTime initTime;
} session;

static QList<session> sessions;

const QString hashing(const QString &string){
    QString hashed = QtBCrypt::hashPassword(string, config.encryption_key);
    return hashed;
}

const QString generateSessionId(const QString &username){
    for (const auto &session : sessions){
        if (session.username == username) return session.id;
    }

    QString combined_string = username + generateRandomString(QRandomGenerator::global()->bounded(100u, 200u));
    session newSession;
    QString hashed = hashing(combined_string);
    newSession.id = hashed;
    newSession.username = username;
    newSession.initTime = QDateTime::currentDateTime();
    sessions.append(newSession);
    return hashed;
}

// const QString decryptAuth(const QString &string){
//     SimpleCrypt crypt(config["encryption_key"].toInteger());
//     return crypt.decryptToString(string);
// }

bool isAuthValid(const QString &id){
    for (const auto &session : sessions){
        if (session.id == id) return true;
    }
    return false;
}

QString getSessionUsername(const QString &id){
    for (const auto &session : sessions){
        if (session.id == id) return session.username;
    }
    return "";
}

bool isAuthValid(const QString &username, const QString &password){
    for (auto &account : config.accounts) {
        if (account.username == username &&
            account.password == password){
            return true;
        }
    }
    return false;
}

bool isAdmin(const QString &id){
    for (const auto &session : sessions){
        if (session.id == id){
            for (auto &account : config.accounts) {
                if (account.username == session.username &&
                    account.is_admin){
                    return true;
                }
            }
        }
    }
    return false;
}

bool isAuthorized(const QHttpServerRequest &request){
    auto id = getAuthCookie(request);
    if (!id)
        return false;

    if (isAuthValid(*id)){
        if (isAdmin(*id))
            return true;
    }
    return false;
}

const QByteArray setAuth(const QString &sessionId){
    QNetworkCookie cookie("sessionId", sessionId.toUtf8());
    cookie.setPath("/");
    cookie.setExpirationDate(QDateTime::currentDateTime().addDays(3));
    //cookie.setHttpOnly(true);
    QByteArray clearCookieHeader = cookie.toRawForm();
    return clearCookieHeader;
}

const QByteArray clearAuth(){
    QNetworkCookie cookie("sessionId", "");
    cookie.setPath("/");
    cookie.setExpirationDate(QDateTime::fromSecsSinceEpoch(0));
    //cookie.setHttpOnly(true);
    QByteArray clearCookieHeader = cookie.toRawForm();
    return clearCookieHeader;
}

QCoro::Task<> checkSessions() {
    uint32_t expire_time = 3*24*60*60; // 3 days
    std::chrono::seconds await_time(60*60);
    while (true) {
        QDateTime current_time = QDateTime::currentDateTime();
        sessions.removeIf([&](const session &s){
            return s.initTime.secsTo(current_time) >= expire_time;
        });

        co_await QCoro::sleepFor(await_time);
    }
}


#endif // AUTH_H
