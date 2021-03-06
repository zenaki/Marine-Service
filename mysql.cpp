#include "mysql.h"

mysql::mysql()
{

}

QSqlDatabase mysql::connect_db(QString name)
{
    QFile db_path(PATH_DB_CONNECTION);
    if (db_path.exists()) {
        QSettings db_sett(PATH_DB_CONNECTION, QSettings::IniFormat);
        host = db_sett.value("HOST").toString();
        db_name = db_sett.value("DATABASE").toString();
        username = db_sett.value("USERNAME").toString();
        password = db_sett.value("PASSWORD").toString();

        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", name);
        db.setHostName(host);
        db.setDatabaseName(db_name);
        db.setUserName(username);
        db.setPassword(password);

        return (QSqlDatabase) db;
    } else {
        host = "localhost";
        db_name = "marine_service";
        username = "root";
        password = "root";

        QSettings db_sett(PATH_DB_CONNECTION, QSettings::IniFormat);
        db_sett.setValue("HOST", host.toUtf8());
        db_sett.setValue("DATABASE", db_name.toUtf8());
        db_sett.setValue("USERNAME", username.toUtf8());
        db_sett.setValue("PASSWORD", password.toUtf8());

        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", name);
        db.setHostName(host);
        db.setDatabaseName(db_name);
        db.setUserName(username);
        db.setPassword(password);

        return (QSqlDatabase) db;
    }
}

void mysql::close(QSqlDatabase db)
{
    QString connection;
    connection = db.connectionName();
    db.close();
    db = QSqlDatabase();
    db.removeDatabase(connection);
}

QStringList mysql::read(QSqlDatabase db, QString query, QStringList parameter)
{
    QStringList result;
    if (db.isValid()) {
        if (!db.isOpen()) {
            db.open();
        }
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));
//        qDebug() << query;
//        q.prepare(query);
        for (int i = 0; i < parameter.length(); i++) {
            if (query.indexOf(":p"+QString::number(i+1)) > 0) {
                query.replace(":p"+QString::number(i+1), "'"+parameter.at(i)+"'");
            }
        }
        if (q.exec(query)) {
            while(q.next()) {
                for (int i = 0; i < q.record().count(); i++) {
                    result.append(q.value(i).toString());
                }
            }
        } else {
            qDebug() << "Cannot read from database with query: \n" << query;
//            qDebug() << "Cannot read from database";
        }
        db.close();
    }
    return result;
}

void mysql:: write(QSqlDatabase db, QString query, QStringList parameter)
{
    if (db.isValid()) {
        if (!db.isOpen()) {
            db.open();
        }
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));
        for (int i = 0; i < parameter.length(); i++) {
            if (query.indexOf(":p"+QString::number(i+1)) > 0) {
                query.replace(":p"+QString::number(i+1), "'"+parameter.at(i)+"'");
            }
        }
//        qDebug() << query;
//        q.prepare(query);
        if (q.exec(query)) {
            qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::Success write to database";
//            qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::Success write to database with query: \n" << query;
//            while(q.next()) {
//                 result.append(q.value(0).toString());
//            }
        } else {
            qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::Cannot write to database with query: \n" << query;
//            qDebug() << "Cannot write to database";
        }
        db.close();
    }
}

void mysql::modem_info(QSqlDatabase db, utama *marine)
{
    if (db.isValid()) {
        if (!db.isOpen()) {
            db.open();
        }
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));
        printf("Ship list :\n");

        int count = 0;
        q.prepare("SELECT id_ship, name, modem_id FROM ship where status = 1");
        if(!q.exec()){
            printf("Initialization                                          [FAILED]\n");
        }
        else{
            while(q.next()){
                marine->kapal[count].id_ship =  q.value(0).toInt();
                strcpy(marine->kapal[count].name, q.value(1).toString().toLatin1());
                strcpy(marine->kapal[count].modem_id, q.value(2).toString().toLatin1());

                count++;
            }
        }

        marine->sum_ship = count;

        printf("Get Num of Ship : %d\n\nList of Ship :\n", marine->sum_ship);
        for (int i = 0; i < marine->sum_ship; i++){
            printf("%d. id_ship : %d , Name : %s , Modem_id : %s\n", i+1, marine->kapal[i].id_ship, marine->kapal[i].name, marine->kapal[i].modem_id);
        }
        printf("Initialization                                          [DONE]\n");
    }
}

void mysql::modem_getway(QSqlDatabase db, account *acc)
{
    if (db.isValid()) {
        if (!db.isOpen()) {
            db.open();
        }
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));
        int n = 0;

        q.prepare("SELECT id, url, access_id, password, next_utc, SIN, MIN, config from gateway");
        if (!q.exec()) {
            printf("Initialization                                          [FAILED]");
        } else {
            while(q.next()){
                QString qStr;

                int id = q.value(0).toInt();
                QString getway = q.value(1).toString() + "get_return_messages.json/";
                QString access_id = q.value(2).toString();
                QString password = q.value(3).toString();
                QDateTime nextutc = q.value(4).toDateTime();
                int SIN = q.value(5).toInt();
                int MIN = q.value(6).toInt();
                QString SinMin = q.value(7).toString();

                qStr.sprintf("%s?access_id=%s&password=%s&start_utc=",
                             getway.toUtf8().data(),
                             access_id.toUtf8().data(),
                             password.toUtf8().data());

                acc->gway[n].id = id;
                strcpy(acc->gway[n].gateway, getway.toLatin1());
                strcpy(acc->gway[n].access_id, access_id.toLatin1());
                strcpy(acc->gway[n].password, password.toLatin1());
                strcpy(acc->gway[n].link, qStr.toLatin1());
                strcpy(acc->gway[n].nextutc, nextutc.toString("yyyy-MM-dd%20hh:mm:ss").toUtf8().data());
                acc->gway[n].SIN = SIN;
                acc->gway[n].MIN = MIN;
                strcpy(acc->gway[n].SinMin, SinMin.toLatin1());
                n++;

                qDebug() << "----------------------------------------";
                qDebug() << "GateWay ID         : " << id;
                qDebug() << "GateWay Address    : " << getway;
                qDebug() << "Access ID          : " << access_id;
                qDebug() << "Password           : " << password;
                qDebug() << "Next UTC           : " << nextutc;
                qDebug() << "SIN                : " << SIN;
                qDebug() << "MIN                : " << MIN;
                qDebug() << "SIN & MIN          : " << SinMin;
                qDebug() << "----------------------------------------";
            }
        }

        acc->sum_getway = n;
    }
}

bool mysql::check_table_is_available(QSqlDatabase db, QString date)
{
    QString query;

    query.clear();
    query = "select * from data_" + date;

    QSqlQuery q(QSqlDatabase::database(db.connectionName()));
    q.prepare(query);

    return q.exec();
}

void mysql::create_tabel_data_harian(QSqlDatabase db, QString date)
{
    QString query;

    query.clear();
    query = "CREATE TABLE if not exists data_"+date+" \
             (id_titik_ukur INT NOT NULL, value FLOAT NOT NULL, id_trip INT NULL DEFAULT NULL, \
             epochtime INT NOT NULL, data_time DATETIME NOT NULL, flag_data INT, PRIMARY KEY (id_titik_ukur, data_time))";
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));
    q.exec(query);
}
