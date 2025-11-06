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
    QJsonArray accounts = config["accounts"].toArray();
    for (const auto &account : std::as_const(accounts)){
        if (account.toObject().value("username").toString() == username)
            return sendStatus("Username already exists", QHttpServerResponder::StatusCode::BadRequest);
    }

    QJsonObject account;
    if (accounts.size() == 0){
        account["username"] = username;
        account["password"] = hashing(password);
        account["isAdmin"] = true;
    }else{
        account["username"] = username;
        account["password"] = hashing(password);
        account["isAdmin"] = false;
    }
    accounts.append(account);
    config["accounts"] = accounts;
    writeConfig();

    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse changeAccount(const QHttpServerRequest &request){
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
    bool isAdmin =  body.value("isAdmin").toBool();

    static const QRegularExpression isPasswordValid(R"(^[ -~]{3,}$)");
    if (!password.isEmpty()){
        QRegularExpressionMatch match = isPasswordValid.match(password);
        if (!match.hasMatch()){
            return sendStatus("Invalid password, password should be more than 5 characters and can only contains ASCII character", QHttpServerResponder::StatusCode::BadRequest);
        }
    }

    QJsonArray accounts = config["accounts"].toArray();
    uint32_t index = 0;
    for (const auto &account : std::as_const(accounts)){
        auto _account = account.toObject();
        if ( _account.value("username").toString() == username){
            if (!password.isEmpty()) _account["password"] = hashing(password);
            _account["isAdmin"] = isAdmin;
            accounts[index] = _account;
            config["accounts"] = accounts;
            writeConfig();
            return sendStatus(QHttpServerResponse::StatusCode::Ok);
        }
        ++index;
    }

    return sendStatus("Unknown error", QHttpServerResponder::StatusCode::BadRequest);
}

QHttpServerResponse deleteAccount(const QHttpServerRequest &request){
    if (auto isInvalid = isContentTypeInvalid(request, "application/json")){
        return std::move(isInvalid.value());
    }

    QJsonDocument doc;

    if (auto isInvalid = parseBody(doc, request.body())){
        return std::move(isInvalid.value());
    }

    QJsonObject body = doc.object();
    QString username = body.value("username").toString();


    QJsonArray oldAccounts = config["accounts"].toArray();
    QJsonArray newAccounts;
    for (const auto &account : std::as_const(oldAccounts)){
        if (account.toObject().value("username").toString() != username){
            newAccounts.append(account);
        }

    }
    config["accounts"] = newAccounts;


    // also remove the account in the source rules if it's exist

    QJsonArray oldFolders = config["folders"].toArray();
    QJsonArray newFolders;

    for (const auto &folder : std::as_const(oldFolders)){
        auto _folder = folder.toObject();
        if (_folder["canSee"].isArray()) _folder["canSee"] = removeAccountAccessSource(username, _folder["canSee"]);
        if (_folder["canUpload"].isArray()) _folder["canUpload"] = removeAccountAccessSource(username, _folder["canUpload"]);
        if (_folder["canDelete"].isArray()) _folder["canDelete"] = removeAccountAccessSource(username, _folder["canDelete"]);
        if (_folder["canDownload"].isArray()) _folder["canDownload"] = removeAccountAccessSource(username, _folder["canDownload"]);
        newFolders.append(_folder);
    }

    QJsonArray oldFiles = config["files"].toArray();
    QJsonArray newFiles;
    for (const auto &file : std::as_const(oldFiles)){
        auto _file = file.toObject();
        if (_file["canSee"].isArray()) _file["canSee"] = removeAccountAccessSource(username, _file["canSee"]);
        if (_file["canDownload"].isArray()) _file["canDownload"] = removeAccountAccessSource(username, _file["canDownload"]);
        newFiles.append(_file);
    }

    config["folders"] = newFolders;
    config["files"] = newFiles;

    writeConfig();
    return sendStatus(QHttpServerResponse::StatusCode::Ok);
}

QHttpServerResponse getAccounts(){
    QJsonArray accounts = config["accounts"].toArray();
    uint32_t index = 0;
    for (const auto &account : std::as_const(accounts)){
        auto _account = account.toObject();
        _account.remove("password");
        accounts[index] = _account;
        ++index;
    }

    QJsonDocument doc(accounts);
    QHttpServerResponse response("application/json; charset=utf-8", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);

    response.setHeaders(createHeaders(.cache = false));
    return response;
}

#endif // ACCOUNTS_H
