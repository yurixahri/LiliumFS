#ifndef CONFIG_H
#define CONFIG_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator64>
#include <QReadWriteLock>
#include "utils/stringUtils.h"
#include "utils/qtbcrypt.h"
#include "utils/types.h"

typedef struct {
    uint16_t port;
    QString encryption_key;
    QList<account_t> accounts;
    QList<directory_t> dirs;
    QList<file_t> files;
    QList<vd_t> vds;

    mutable QReadWriteLock lock;
} config_t;

extern config_t config;

// extern QJsonObject config;

bool writeConfig();
bool getConfig();



#endif // CONFIG_H
