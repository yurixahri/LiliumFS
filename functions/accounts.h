#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "httpHelper.h"
#include "utils/config.h"
#include "utils/auth.h"
#include "functions/sources.h"

QHttpServerResponse addAccount(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    static const QRegularExpression isUsernameValid(R"(^\w{3,}$)"); // username can only contains character, number and underscore
    static const QRegularExpression isPasswordValid(R"(^[ -~]{6,}$)"); // password can only contains ASCII character
    QString username = body.value("username").toString();
    QString password = body.value("password").toString();

    QRegularExpressionMatch match = isUsernameValid.match(username);
    if (!match.hasMatch()){
        return sendStatus("Invalid username, username should be more than 2 characters and can only contains character, number and underscore", QHttpServerResponder::StatusCode::BadRequest);
    }

    match = isPasswordValid.match(password);
    if (!match.hasMatch()){
        return sendStatus("Invalid password, password should be more than 5 characters and can only contains ASCII character", QHttpServerResponder::StatusCode::BadRequest);
    }

    //checking duplicate account
    for (auto &account : config.accounts){
        if (account.username == username)
            return sendStatus("Username already exists", QHttpServerResponder::StatusCode::BadRequest);
    }

    account_t new_account;
    if (config.accounts.size() == 0){
        new_account.is_admin = true;
    }else{
        new_account.is_admin = false;
    }
    new_account.username = username;
    new_account.password = hashing(password);

    config.accounts << new_account;
    writeConfig();

    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse changeAccount(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString username = body.value("username").toString();
    QString password = body.value("password").toString();
    bool is_admin =  body.value("is_admin").toBool();

    static const QRegularExpression isPasswordValid(R"(^[ -~]{3,}$)");
    if (!password.isEmpty()){
        QRegularExpressionMatch match = isPasswordValid.match(password);
        if (!match.hasMatch()){
            return sendStatus("Invalid password, password should be more than 5 characters and can only contains ASCII character", QHttpServerResponder::StatusCode::BadRequest);
        }
    }

    for (auto &account : config.accounts){
        if (account.username == username){
            if (!password.isEmpty()) account.password = hashing(password);
            account.is_admin = is_admin;
            writeConfig();
            return sendStatus(QHttpServerResponse::StatusCode::Ok);
        }
    }

    return sendStatus("Unknown error", QHttpServerResponder::StatusCode::BadRequest);
}

QHttpServerResponse deleteAccount(const QHttpServerRequest &request){
    QWriteLocker locker(&config.lock);

    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString username = body.value("username").toString();

    config.accounts.removeIf([&](const account_t &account){
        return account.username == username;
    });
    // also remove the account in the source rules if it's exist

    for (auto &dir : config.dirs){
        removeAccountAccessSource(username, dir.can_see);
        removeAccountAccessSource(username, dir.can_download);
        removeAccountAccessSource(username, dir.can_upload);
        removeAccountAccessSource(username, dir.can_delete);
    }

    for (auto &file : config.files){
        removeAccountAccessSource(username, file.can_see);
        removeAccountAccessSource(username, file.can_download);
    }

    for (auto &vd : config.vds) {
        removeAccountAccessSource(username, vd.can_see);

        for (auto &dir : vd.dirs){
            removeAccountAccessSource(username, dir.can_see);
            removeAccountAccessSource(username, dir.can_download);
            removeAccountAccessSource(username, dir.can_upload);
            removeAccountAccessSource(username, dir.can_delete);
        }

        for (auto &file : vd.files){
            removeAccountAccessSource(username, file.can_see);
            removeAccountAccessSource(username, file.can_download);
        }
    }

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse getAccounts(){
    QReadLocker locker(&config.lock);

    QVariantList accounts;
    for(const auto &account : config.accounts) accounts << account_to_map_client(account);

    QJsonDocument doc = QJsonDocument::fromVariant(accounts);
    QHttpServerResponse response("application/json; charset=utf-8", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);

    response.setHeaders(createHeaders(.cache = false));
    return response;
}

#endif // ACCOUNTS_H
