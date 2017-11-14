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
                 result.append(q.value(0).toString());
            }
        } else {
            qDebug() << "Cannot read from database with query: \n" << query;
        }
        db.close();
    }
    return result;
}

void mysql::write(QSqlDatabase db, QString query, QStringList parameter)
{
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
//            while(q.next()) {
//                 result.append(q.value(0).toString());
//            }
        } else {
            qDebug() << "Cannot write to database with query: \n" << query;
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

        q.prepare("SELECT id, url, access_id, password, next_utc, SIN, MIN from gateway");
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

                n++;
            }
        }

        acc->sum_getway = n;
    }
}
