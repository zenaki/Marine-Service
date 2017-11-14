#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include "config.h"
#include "mysql.h"

class worker : public QObject
{
    Q_OBJECT
public:
    explicit worker(QObject *parent = 0);

    struct utama *marine;
    struct account *acc;

    config cfg;
    mysql database;
    QSqlDatabase db;

    QString plugin_path;
    int interval;

    QJsonObject ObjectFromString(QString in);
    void parse_kureyGeo(QJsonObject obj);
    void parse_imaniPrima(QJsonObject obj);
    void parse(QJsonObject obj);
public slots:
    void doWork();
};

#endif // WORKER_H
