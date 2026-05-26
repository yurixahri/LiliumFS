#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QVariant>
#include <QList>

typedef struct {
    QString src;
    QString name;
    QVariant can_see;
    QVariant can_download;
    QVariant can_upload;
    QVariant can_delete;
    QString time;
} directory_t;

typedef struct {
    QString src;
    QString name;
    QVariant can_see;
    QVariant can_download;
    QString time;
    qint64 size;
} file_t;

typedef struct {
    QString name;
    QList<directory_t> dirs;
    QList<file_t> files;
    QVariant can_see;
    QVariant can_download;
} vd_t; //virtual directory

typedef struct {
    QString username;
    QString password;
    bool is_admin;
} account_t;

inline account_t map_to_account(const QVariantMap &m){
    return account_t{m["username"].toString(), m["password"].toString(), m["is_admin"].toBool()};
}

inline directory_t map_to_dir(const QVariantMap &m){
    return directory_t{m["src"].toString(), m["name"].toString(), m["can_see"], m["can_download"], m["can_upload"], m["can_delete"]};
}

inline file_t map_to_file(const QVariantMap &m){
    return file_t{m["src"].toString(), m["name"].toString(), m["can_see"], m["can_download"]};
}

inline vd_t map_to_vd(const QVariantMap &m) {
    vd_t vd;
    // Parse directories inside the VD
    for (const QVariant &d_var : m["dirs"].toList()) {
        QVariantMap dm = d_var.toMap();
        vd.dirs << directory_t{
            dm["src"].toString(),
            dm["name"].toString(),
            dm["can_see"],
            dm["can_download"],
            dm["can_upload"],
            dm["can_delete"]
        };
    }
    // Parse files inside the VD
    for (const QVariant &f_var : m["files"].toList()) {
        QVariantMap fm = f_var.toMap();
        vd.files << file_t{
            fm["src"].toString(),
            fm["name"].toString(),
            fm["can_see"],
            fm["can_download"]
        };
    }

    vd.name = m["name"].toString();
    vd.can_see = m["can_see"];
    vd.can_download = m["can_download"];
    return vd;
}

inline QVariantMap account_to_map(const account_t &a) {
    return {{"username", a.username}, {"password", a.password}, {"is_admin", a.is_admin}};
}

inline QVariantMap dir_to_map(const directory_t &d) {
    return {{"src", d.src}, {"name", d.name}, {"can_see", d.can_see},
            {"can_download", d.can_download}, {"can_upload", d.can_upload}, {"can_delete", d.can_delete}};
}

inline QVariantMap file_to_map(const file_t &f) {
    return {{"src", f.src}, {"name", f.name}, {"can_see", f.can_see}, {"can_download", f.can_download}};
}

inline QVariantMap vd_to_map(const vd_t &vd) {
    QVariantList dirs, files;
    for (const auto &d : vd.dirs) dirs << dir_to_map(d);
    for (const auto &f : vd.files) files << file_to_map(f);

    return {
        {"name", vd.name},
        {"dirs", dirs},
        {"files", files},
        {"can_see", vd.can_see},
        {"can_download", vd.can_download}
    };
}

inline QVariantMap dir_to_map_client(const directory_t &d) {
    return {{"name", d.name}, {"time", d.time}};
}

inline QVariantMap file_to_map_client(const file_t &f) {
    return {{"name", f.name}, {"time", f.time}, {"size", f.size}};
}

inline QVariantMap account_to_map_client(const account_t &a) {
    return {{"username", a.username}, {"is_admin", a.is_admin}};
}


#endif // TYPES_H
