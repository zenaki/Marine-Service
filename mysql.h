#ifndef MYSQL_H
#define MYSQL_H

#include "utama.h"

class mysql
{
public:
    mysql();

    QString host;
    QString db_name;
    QString username;
    QString password;

    QSqlDatabase connect_db(QString name);
    void close(QSqlDatabase db);

    QStringList read(QSqlDatabase db, QString query, QStringList parameter);
    void write(QSqlDatabase db, QString query, QStringList parameter);
    void modem_info(QSqlDatabase db, utama *marine);
    void modem_getway(QSqlDatabase db, account *acc);
};

#endif // MYSQL_H
