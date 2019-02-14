#include "worker.h"

worker::worker(QObject *parent) : QObject(parent)
{   
    db = database.connect_db("Marine_Service");
    marine = (struct utama *) malloc( sizeof (struct utama));
    memset((char *) marine, 0, sizeof(struct utama));

    acc = (struct account *) malloc ( sizeof (struct account));
    memset((char *) acc, 0, sizeof(struct account));

    database.modem_info(db, marine);
    database.modem_getway(db, acc);
//    QStringList list_titik_ukur;
//    for (int i = 0; i < marine->sum_ship; i++) {
//        list_titik_ukur = database.read(db,
//                                                    "select urutan_data_monita, id_tu "
//                                                    "from marine_1.parsing_ref "
//                                                    "where id_ship = :p1 "
//                                                    "order by urutan_data_monita asc",
//                                                    QStringList(QString::number(marine->kapal[i].id_ship)));
//        marine->kapal[i].titik_ukur = list_titik_ukur;
//    }
    QStringList conf = cfg.read("CONFIG");
    plugin_path = conf.at(0);
    interval = conf.at(1).toInt();

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(interval);
    doWork();
}

void worker::doWork()
{
    for (int i = 0; i < acc->sum_getway; i++) {
        QProcess proc;
        QString command;
        command.sprintf("./%s -g %s -aid %s -pwd %s -sm %s -s %s -t 60000",
                        plugin_path.toUtf8().data(),
                        acc->gway[i].gateway,
                        acc->gway[i].access_id,
                        acc->gway[i].password,
                        acc->gway[i].SinMin,
                        acc->gway[i].nextutc);
//        command.sprintf("./%s -g %s -aid %s -pwd %s -sm %s#%s -s %s -t 60000",
//                        plugin_path.toUtf8().data(),
//                        acc->gway[i].gateway,
//                        acc->gway[i].access_id,
//                        acc->gway[i].password,
//                        QString::number(acc->gway[i].SIN).toUtf8().data(),
//                        QString::number(acc->gway[i].MIN).toUtf8().data(),
//                        acc->gway[i].nextutc);
//        command.sprintf("./%s -g %s -aid %s -pwd %s -sm %s#%s -s 2017-11-14#00:00:00 -t 60000",
//                        plugin_path.toUtf8().data(),
//                        acc->gway[i].gateway,
//                        acc->gway[i].access_id,
//                        acc->gway[i].password,
//                        QString::number(acc->gway[i].SIN).toUtf8().data(),
//                        QString::number(acc->gway[i].MIN).toUtf8().data());
        command.replace("%20", "#");
//        printf("%s::%s\n\n",
//               QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz").toUtf8().data(),
//               command.toUtf8().data());
        qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + command;
        proc.start(command);
        proc.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
        QString output(proc.readAllStandardOutput());
        QJsonObject obj = this->ObjectFromString(output);

        if (!obj.value("skywave").isUndefined()) {
            if (acc->gway[i].id == 1) {
//                this->parse_kureyGeo(obj);
                this->parse(obj, i);
            } else if (acc->gway[i].id == 2) {
//                this->parse_imaniPrima(obj);
                this->parse(obj, i);
            }
        }
    }
}

QJsonObject worker::ObjectFromString(QString in)
{
    QJsonObject obj;
    QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

    // check validity of the document
    if (!doc.isNull()) {
        if (doc.isObject()) {
            obj = doc.object();
        }
    }

    return obj;
}

void worker::parse(QJsonObject obj, int index)
{
    QStringList data, list_waktu;
    if (obj.value("skywave").isArray()) {
        QJsonArray array = obj.value("skywave").toArray();
        foreach (const QJsonValue & v, array) {
            bool bWaktu = false;
            QString date = QDateTime::fromString(v.toObject().value("ReceiveUTC").toString(), "yyyy-MM-dd HH:mm:ss").toString("yyyyMMdd");
            if (list_waktu.length() > 0) {
                for (int i = 0; i < list_waktu.length(); i++) {
                    if (list_waktu.at(i) == date) {
                        bWaktu = true;
                        break;
                    }
                }
            }
            if (!bWaktu) list_waktu.append(date);
        }
        for (int i = 0; i < list_waktu.length(); i++) {
            qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + "List Waktu " + QString::number(i+1) + " : " + list_waktu.at(i);
        }
//        return;
        for (int k = 0; k < list_waktu.length(); k++) {
            QDateTime timeTemp = QDateTime::fromString(list_waktu.at(k), "yyyyMMdd");
            if (!database.check_table_is_available(db, timeTemp.toString("yyyyMMdd"))) {
                database.create_tabel_data_harian(db, timeTemp.toString("yyyyMMdd"));
            }
            foreach (const QJsonValue & v, array) {
                if (v.toObject().value("SIN").toInt() == 128 && v.toObject().value("MIN").toInt() == 1) {
                    if (!v.toObject().value("Payload").isNull()) {
                        QStringList payload;
                        foreach (const QJsonValue & vPayLoad, v.toObject().value("Payload").toArray()) {
                            payload.append(vPayLoad.toString());
                        }
                        for (int i = 0; i < marine->sum_ship; i++) {
                            if (marine->kapal[i].modem_id == v.toObject().value("MobileID").toString()) {
                                QStringList list_tu = database.read(db,
                                                                    "select urutan_data_monita, id_tu "
                                                                    "from marine_1.parsing_ref "
                                                                    "where id_ship = :p1 and urutan_data_monita <> 0 "
                                                                    "order by urutan_data_monita asc",
                                                                    QStringList(QString::number(marine->kapal[i].id_ship)));;
//        "REPLACE INTO data(id_titik_ukur, value, id_trip, epochtime, data_time) VALUES(:id_titik_ukur, :value, :id_trip, :epochtime, :data_time)"
                                if (list_tu.length() > 0 && payload.length() > 0) {
                                    qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + "SIN 128 MIN 1";
                                    qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + "titik ukur           value           time";
                                    if (list_tu.length()/2 > payload.length()) {
                                        for (int j = 0; j < payload.length(); j++) {
                                            if (list_tu.at(j*2).toInt() == j+1) {
                                                QDateTime time = QDateTime::fromString(v.toObject().value("ReceiveUTC").toString(), "yyyy-MM-dd HH:mm:ss");
                                                if (timeTemp.toString("yyyyMMdd") == time.toString("yyyyMMdd")) {
                                                    time.setTimeSpec(Qt::UTC);
                                                    QDateTime local = time.toLocalTime();
                                                    data.append("('"+list_tu.at((j*2)+1)+
                                                                "','"+payload.at(j)+
                                                                "','0','"+QString::number(time.toTime_t())+
                                                                "','"+local.toString("yyyy-MM-dd HH:mm:ss")+
                                                                "','"+QString::number(FLAG_ENGINE_DATA)+"')");
                                                    qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + list_tu.at((j*2)+1) + "           " + payload.at(j) + "          " + time.toString("yyyy-MM-dd HH:mm:ss");
                                                }
                                            }
                                        }
                                    } else {
                                        for (int j = 0; j < list_tu.length(); j+=2) {
                                            QDateTime time = QDateTime::fromString(v.toObject().value("ReceiveUTC").toString(), "yyyy-MM-dd HH:mm:ss");
                                            if (timeTemp.toString("yyyyMMdd") == time.toString("yyyyMMdd")) {
                                                time.setTimeSpec(Qt::UTC);
                                                QDateTime local = time.toLocalTime();
                                                data.append("('"+list_tu.at(j+1)+
                                                            "','"+payload.at(list_tu.at(j).toInt()-1)+
                                                            "','0','"+QString::number(time.toTime_t())+
                                                            "','"+local.toString("yyyy-MM-dd HH:mm:ss")+
                                                            "','"+QString::number(FLAG_ENGINE_DATA)+"')");
                                                qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + list_tu.at(j+1) + "           " + payload.at(list_tu.at(j).toInt()-1) + "          " + time.toString("yyyy-MM-dd HH:mm:ss");
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else if (v.toObject().value("SIN").toInt() == 19 && v.toObject().value("MIN").toInt() == 4) {
                    if (!v.toObject().value("Payload").isNull()) {
                        QStringList payload;
                        foreach (const QJsonValue & vPayLoad, v.toObject().value("Payload").toArray()) {
                            payload.append(vPayLoad.toString());
                        }
                        for (int i = 0; i < marine->sum_ship; i++) {
                            if (marine->kapal[i].modem_id == v.toObject().value("MobileID").toString()) {
                                QStringList list_tu = database.read(db,
                                                                    "select urutan_dua, id_tu "
                                                                    "from marine_1.parsing_ref "
                                                                    "where id_ship = :p1 and urutan_dua <> 0 "
                                                                    "order by urutan_dua asc",
                                                                    QStringList(QString::number(marine->kapal[i].id_ship)));;
//        "REPLACE INTO data(id_titik_ukur, value, id_trip, epochtime, data_time) VALUES(:id_titik_ukur, :value, :id_trip, :epochtime, :data_time)"
                                if (list_tu.length() > 0 && payload.length() > 0) {
                                    qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + "SIN 19 MIN 4";
                                    qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + "titik ukur           value           time";
                                    if (list_tu.length()/2 > payload.length()) {
                                        for (int j = 0; j < payload.length(); j++) {
                                            if (list_tu.at(j*2).toInt() == j+1) {
                                                QDateTime time = QDateTime::fromString(v.toObject().value("ReceiveUTC").toString(), "yyyy-MM-dd HH:mm:ss");
                                                time.setTimeSpec(Qt::UTC);
                                                QDateTime local = time.toLocalTime();
                                                data.append("('"+list_tu.at((j*2)+1)+
                                                            "','"+payload.at(j)+
                                                            "','0','"+QString::number(time.toTime_t())+
                                                            "','"+local.toString("yyyy-MM-dd HH:mm:ss")+
                                                            "','"+QString::number(FLAG_TRACKING_DATA)+"')");
                                                qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + list_tu.at((j*2)+1) + "           " + payload.at(j) + "          " + time.toString("yyyy-MM-dd HH:mm:ss");
                                                if (!database.check_table_is_available(db, time.toString("yyyyMMdd"))) {
                                                    database.create_tabel_data_harian(db, time.toString("yyyyMMdd"));
                                                }
                                            }
                                        }
                                    } else {
                                        for (int j = 0; j < list_tu.length(); j+=2) {
                                            QDateTime time = QDateTime::fromString(v.toObject().value("ReceiveUTC").toString(), "yyyy-MM-dd HH:mm:ss");
                                            time.setTimeSpec(Qt::UTC);
                                            QDateTime local = time.toLocalTime();
                                            data.append("('"+list_tu.at(j+1)+
                                                        "','"+payload.at(list_tu.at(j).toInt()-1)+
                                                        "','0','"+QString::number(time.toTime_t())+
                                                        "','"+local.toString("yyyy-MM-dd HH:mm:ss")+
                                                        "','"+QString::number(FLAG_TRACKING_DATA)+"')");
                                            qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + list_tu.at(j+1) + "           " + payload.at(list_tu.at(j).toInt()-1) + "          " + time.toString("yyyy-MM-dd HH:mm:ss");
                                            if (!database.check_table_is_available(db, time.toString("yyyyMMdd"))) {
                                                database.create_tabel_data_harian(db, time.toString("yyyyMMdd"));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (data.length() > 0) {
//                printf("jumlah data baru = %d\n",data.length());
                qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + "jumlah data baru = "+ QString::number(data.length());
                QString query = "replace into data (id_titik_ukur, value, id_trip, epochtime, data_time, flag_data) values ";
                for (int i = 0; i < data.length(); i++) {
                    if (i == data.length()-1) {
                        query = query + data.at(i) + ";";
                    } else {
                        query = query + data.at(i) + ",";
                    }
                }
//                qDebug() << query;
                qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + "write ke table data ...";
                database.write(db,query,QStringList());
                query = "replace into data_"+timeTemp.toString("yyyyMMdd")+" (id_titik_ukur, value, id_trip, epochtime, data_time, flag_data) values ";
                for (int i = 0; i < data.length(); i++) {
                    if (i == data.length()-1) {
                        query = query + data.at(i) + ";";
                    } else {
                        query = query + data.at(i) + ",";
                    }
                }
//                qDebug() << query;
                qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + "write ke table data_"+timeTemp.toString("yyyyMMdd")+" ...";
                database.write(db,query,QStringList());
                qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + "selesai ...";
            }
        }
        if (!obj.value("NextStartUTC").isUndefined()) {
            QStringList parameter;
            parameter.append(obj.value("NextStartUTC").toString());
            parameter.append(QString::number(acc->gway[index].id));
            database.write(db,"update gateway set next_utc = :p1 where id = :p2", parameter);
//            printf("next utc = %s\n",obj.value("NextStartUTC").toString().toUtf8().data());
            qDebug() << QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz") + "::" + "next utc = "+obj.value("NextStartUTC").toString();
            strcpy(acc->gway[index].nextutc, QDateTime::fromString(obj.value("NextStartUTC").toString(), "yyyy-MM-dd HH:mm:ss").toString("yyyy-MM-dd%20HH:mm:ss").toUtf8().data());
        }
    }
}
