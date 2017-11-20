#ifndef UTAMA_H
#define UTAMA_H

#include <QDebug>
#include <QTimer>
#include <QProcess>

#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QDateTime>

#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#define PATH_DB_CONNECTION      ".MarineConfig/marine-db.dbe"
#define PATH_CONFIGURATION_JSON ".MarineConfig/marine-cfg.json"
#define PATH_CONFIGURATION_BIN  ".MarineConfig/marine-cfg.dat"
#define PATH_MONITA             ".MarineConfig/"

struct ship{
    int id_ship;
    char name[32];
    char modem_id[32];
    QStringList titik_ukur;
};

struct utama{
    int sum_ship;
    struct ship kapal[50]; /* masih hardoce untuk alokasi 50 kapal */
};

struct getway{
    int id;
    char gateway[512];
    char access_id[32];
    char password[32];
    char link[512];
    char nextutc[32];
    int SIN;
    int MIN;
};

struct account{
    int sum_getway;
    struct getway gway[10];
};

#endif // UTAMA_H
