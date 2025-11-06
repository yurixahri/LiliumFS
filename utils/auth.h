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

static QVector<session> sessions;

const QString hashing(const QString &string){
    QString hashed = QtBCrypt::hashPassword(string, config["encryption_key"].toString());
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
    QJsonArray accounts = config["accounts"].toArray();
    for (const auto &account : std::as_const(accounts)) {
        if (account.toObject().value("username").toString() == username &&
            account.toObject().value("password").toString() == password){
            return true;
        }
    }
    return false;
}

bool isAdmin(const QString &id){
    for (const auto &session : sessions){
        if (session.id == id){
            QJsonArray accounts = config["accounts"].toArray();
            for (const auto &account : std::as_const(accounts)) {
                if (account.toObject().value("username").toString() == session.username &&
                    account.toObject().value("isAdmin").toBool()){
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
    uint32_t expireTime = 3*24*60*60; // 3 days
    std::chrono::seconds awaitTime(60*60);
    while (true) {
        QDateTime currentTime = QDateTime::currentDateTime();
        QVector<session> newSessions;
        for (const auto &session : sessions){
            if (session.initTime.secsTo(currentTime) < expireTime)
                newSessions.push_back(session);
        }
        sessions = newSessions;
        co_await QCoro::sleepFor(awaitTime);
    }
}


#endif // AUTH_H
