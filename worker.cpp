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
        command.sprintf("./%s -g %s -aid %s -pwd %s -sm %s#%s -s %s -t 60000",
                        plugin_path.toUtf8().data(),
                        acc->gway[i].gateway,
                        acc->gway[i].access_id,
                        acc->gway[i].password,
                        QString::number(acc->gway[i].SIN).toUtf8().data(),
                        QString::number(acc->gway[i].MIN).toUtf8().data(),
                        acc->gway[i].nextutc);
//        command.sprintf("./%s -g %s -aid %s -pwd %s -sm %s#%s -s 2017-11-14#00:00:00 -t 60000",
//                        plugin_path.toUtf8().data(),
//                        acc->gway[i].gateway,
//                        acc->gway[i].access_id,
//                        acc->gway[i].password,
//                        QString::number(acc->gway[i].SIN).toUtf8().data(),
//                        QString::number(acc->gway[i].MIN).toUtf8().data());
        command.replace("%20", "#");
        printf("%s::%s",
               QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss:zzz").toUtf8().data(),
               command.toUtf8().data());
        proc.start(command);
        proc.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
        QString output(proc.readAllStandardOutput());
        QJsonObject obj = this->ObjectFromString(output);

        if (!obj.value("skywave").isUndefined()) {
            if (acc->gway[i].id == 1) {
//                this->parse_kureyGeo(obj);
                this->parse(obj);
            } else if (acc->gway[i].id == 2) {
//                this->parse_imaniPrima(obj);
                this->parse(obj);
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

void worker::parse(QJsonObject obj)
{
    QStringList data;
    if (obj.value("skywave").isArray()) {
        QJsonArray array = obj.value("skywave").toArray();
        foreach (const QJsonValue & v, array) {
            if (!v.toObject().value("Payload").isNull()) {
                QStringList payload;
                foreach (const QJsonValue & vPayLoad, v.toObject().value("Payload").toArray()) {
                    payload.append(vPayLoad.toString());
                }
                for (int i = 0; i < marine->sum_ship; i++) {
                    if (marine->kapal[i].modem_id == v.toObject().value("MobileID").toString()) {
                        QStringList list_tu = database.read(db,
                                                            "select id_tu "
                                                            "from marine_1.parsing_ref "
                                                            "where id_ship = :p1 "
                                                            "order by urutan_data_monita asc",
                                                            QStringList(QString::number(marine->kapal[i].id_ship)));
//"REPLACE INTO data(id_titik_ukur, value, id_trip, epochtime, data_time) VALUES(:id_titik_ukur, :value, :id_trip, :epochtime, :data_time)"
                        if (list_tu.length() > payload.length()) {
                            for (int j = 0; j < payload.length(); j++) {
                                QDateTime time = QDateTime::fromString(v.toObject().value("ReceiveUTC").toString(), "yyyy-MM-dd HH:mm:ss");
                                time.setTimeSpec(Qt::UTC);
                                QDateTime local = time.toLocalTime();
                                data.append("("+list_tu.at(j)+
                                            ","+payload.at(j)+
                                            ",0,"+QString::number(time.toTime_t())+
                                            ","+local.toString("yyyy-MM-dd HH:mm:ss")+")");
                            }
                        } else {
                            for (int j = 0; j < list_tu.length(); j++) {
                                QDateTime time = QDateTime::fromString(v.toObject().value("ReceiveUTC").toString(), "yyyy-MM-dd HH:mm:ss");
                                time.setTimeSpec(Qt::UTC);
                                QDateTime local = time.toLocalTime();
                                data.append("('"+list_tu.at(j)+
                                            "','"+payload.at(j)+
                                            "','0','"+QString::number(time.toTime_t())+
                                            "','"+local.toString("yyyy-MM-dd HH:mm:ss")+"')");
                            }
                        }
                    }
                }
            }
        }
    }
    if (data.length() > 0) {
        QString query = "replace into data_test (id_titik_ukur, value, id_trip, epochtime, data_time) values ";
        for (int i = 0; i < data.length(); i++) {
            if (i == data.length()-1) {
                query = query + data.at(i) + ";";
            } else {
                query = query + data.at(i) + ",";
            }
        }
        qDebug() << query;
        database.write(db,query,QStringList());
    }
}

void worker::parse_kureyGeo(QJsonObject obj)
{
    if (obj.value("skywave").isArray()) {
        QJsonArray array = obj.value("skywave").toArray();
        foreach (const QJsonValue & v, array) {
            if (!v.toObject().value("Payload").isNull()) {
                for (int i = 0; i < marine->sum_ship; i++) {
                    if (marine->kapal[i].modem_id == v.toObject().value("MobileID").toString()) {
                        QStringList list_tu = database.read(db,
                                                            "select id_tu "
                                                            "from marine_1.parsing_ref "
                                                            "where id_ship = :p1 "
                                                            "order by urutan_data_monita asc",
                                                            QStringList(QString::number(marine->kapal[i].id_ship)));
                        qDebug() << "Test";
                    }
                }
            }
        }
    }
    qDebug() << "Test";
//    int cnt = 0;
//    int cnt_tu = 1;
//    int n;

//    int SIN;

//    int epochtime;
//    QString dat_time;

//    QString MobileID;
//    QString MessageUTC;

//    QXmlStreamReader xml;

//    xml.clear();
//    xml.addData(skyw);

//    while(!xml.atEnd() &&  !xml.hasError()){
//        QXmlStreamReader::TokenType token = xml.readNext();
//        if(token == QXmlStreamReader::StartElement){
//            bool id_match = false;

//            if (xml.name() == "MessageUTC"){
//                MessageUTC.sprintf("%s", xml.readElementText().toUtf8().data());
//                save.update_next_utc_gateway(q, MessageUTC, id_gateway);

//                strcpy(acc->gway[id_gateway-1].nextutc, MessageUTC.toLatin1());
//            }

//            if (xml.name() == "SIN"){
//                SIN = xml.readElementText().toInt();
//            }

//            if (xml.name() == "MobileID"){
//                MobileID.sprintf("%s", xml.readElementText().toUtf8().data());
//            }

//            /* Filtering Mobile ID */
//            for(int i = 0; i < marine->sum_ship; i++){
//                if(marine->kapal[i].modem_id == MobileID){
//                    n = i;
//                    id_match = true;
//                }
//            }

//            if(id_match){
//                //save.update_next_utc(q, MessageUTC, marine->kapal[n].id_ship);

//                if (xml.name() == "Payload"){
//                    QXmlStreamAttributes attributes = xml.attributes();

//                    QString name = attributes.value("Name").toString();
//                    QString sin = attributes.value("SIN").toString();
//                    QString min = attributes.value("MIN").toString();

//                    cnt = 0;
//                    cnt_tu = 1;
//                }
//                if (xml.name() == "Field"){
//                    QXmlStreamAttributes attributes = xml.attributes();

//                    QString name = attributes.value("Name").toString();
//                    int value = attributes.value("Value").toString().toInt();

//                    float data_f = *(float *) &value;

//                    if (cnt == 0){
//                        epochtime = (int) data_f;

//                        const QDateTime time = QDateTime::fromTime_t((int)data_f);
//                        dat_time = time.toString("yyyy-MM-dd hh:mm:ss").toLocal8Bit().data();

//                        cnt++;
//                    }
//                    else{
//                        q->clear();
//                        int id_tu = get.id_tu_ship(q, marine->kapal[n].id_ship, cnt_tu);
//                        if (id_tu != 0){
//                            q->clear();
//                            printf("%d --> %.2f\n", id_tu, data_f);
//                            save.data(q, data_f, id_tu, 0, epochtime, dat_time);
//                        }
//                        else{
//                            printf("\nbelum di set parsing refnya");
//                        }
//                        cnt_tu++;
//                    }
//                }
//            }
//        }
//    }
}

void worker::parse_imaniPrima(QJsonObject obj)
{
    if (obj.value("skywave").isArray()) {
        QJsonArray array = obj.value("skywave").toArray();
        foreach (const QJsonValue & v, array) {
            if (!v.toObject().value("Payload").isNull()) {
                QStringList payload;
                foreach (const QJsonValue & vPayLoad, v.toObject().value("Payload").toArray()) {
                    payload.append(vPayLoad.toString());
                }
                for (int i = 0; i < marine->sum_ship; i++) {
                    if (marine->kapal[i].modem_id == v.toObject().value("MobileID").toString()) {
                        QStringList list_tu = database.read(db,
                                                            "select id_tu "
                                                            "from marine_1.parsing_ref "
                                                            "where id_ship = :p1 "
                                                            "order by urutan_data_monita asc",
                                                            QStringList(QString::number(marine->kapal[i].id_ship)));
                        if (list_tu.length() > payload.length()) {
                            for (int j = 0; j < payload.length(); j++) {
//                                "REPLACE INTO data(id_titik_ukur, value, id_trip, epochtime, data_time) VALUES(:id_titik_ukur, :value, :id_trip, :epochtime, :data_time)"
                            }
                        } else {
                            for (int j = 0; j < list_tu.length(); j++) {

                            }
                        }
                    }
                }
            }
        }
    }
    qDebug() << "Test";
//    int cnt = 0;
//    int cnt_tu = 1;
//    int n;

//    int SIN;

//    int epochtime;
//    QString dat_time;

//    QString MobileID;
//    QString MessageUTC;
//    QString RawPayload;

//    QString decode;
//    QString bin;
//    QString f_5c32g;

//    QXmlStreamReader xml;

//    xml.clear();
//    xml.addData(skyw);

//    while(!xml.atEnd() &&  !xml.hasError()){
//        QXmlStreamReader::TokenType token = xml.readNext();
//        if(token == QXmlStreamReader::StartElement){
//            bool id_match = false;

//            if (xml.name() == "MessageUTC"){
//                MessageUTC.sprintf("%s", xml.readElementText().toUtf8().data());
//                save.update_next_utc_gateway(q, MessageUTC, id_gateway);

//                strcpy(acc->gway[id_gateway-1].nextutc, MessageUTC.toLatin1());
//            }

//            if (xml.name() == "SIN"){
//                SIN = xml.readElementText().toInt();
//            }

//            if (xml.name() == "MobileID"){
//                MobileID.sprintf("%s", xml.readElementText().toUtf8().data());
//            }

//            /* Filtering Mobile ID */
//            for(int i = 0; i < marine->sum_ship; i++){
//                if(marine->kapal[i].modem_id == MobileID){
//                    n = i;
//                    id_match = true;
//                }
//            }

//            if(id_match){
//                if(xml.name() == "RawPayload"){
//                    RawPayload.clear();
//                    RawPayload.sprintf("%s", xml.readElementText().toUtf8().data());

//                    decode.clear();
//                    decode = parse.decode_base64(RawPayload);

//                    bin.clear();
//                    bin = parse.hex_to_bin_conversion(decode);

//                    f_5c32g.clear();
//                    f_5c32g = parse.format_5cut_32get(bin);

//                    parse.parse_data(q, f_5c32g, marine->kapal[n].id_ship);
//                }
//            }
//        }
//    }
}
