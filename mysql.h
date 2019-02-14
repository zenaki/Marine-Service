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

    bool check_table_is_available(QSqlDatabase db, QString date);
    void create_tabel_data_harian(QSqlDatabase db, QString date);
};

#endif // MYSQL_H
