#include "api_marine.h"

api_marine::api_marine()
{
    db = database.connect_db("API_Marine");
}

void api_marine::onGet(qttp::HttpData &data)
{
  QJsonValue api_key = data.getRequest().getJson()["api_key"];
//  perl -e 'print crypt("admin","fde50ab043b064dea0589385820de7e1")'
  QJsonValue start_date = data.getRequest().getJson()["start_date"];
  QJsonValue end_date = data.getRequest().getJson()["end_date"];
  QJsonValue ship_name = data.getRequest().getJson()["ship_name"];

  qttp::HttpResponse& response = data.getResponse();
  QJsonObject& json = response.getJson();
  QStringList parameter = data.getRequest().getQuery().toString().split("&");
  QString query;
  QStringList result;
  if (ship_name.toString() == "all") ship_name = QJsonValue("");
  if(api_key.isString()) {
      query = "select username from user where api_key = '"+ api_key.toString() + "' limit 1";
      result = database.read(db,query,QStringList());
      if (result.length() > 0) {
          QString epochtime;
          if (start_date.isString() && end_date.isString()) {
              epochtime = "dt.epochtime between "
                          "(unix_timestamp('"+start_date.toString()+"')) and "
                          "(unix_timestamp('"+end_date.toString()+"')) ";
          } else if (start_date.isString() && !end_date.isString()) {
              epochtime = "dt.epochtime between "
                          "(unix_timestamp('"+start_date.toString()+"')) and "
                          "(unix_timestamp()) ";
          } else {
              epochtime = "dt.epochtime between "
                          "(unix_timestamp()-86400) and "
                          "(unix_timestamp()) ";
          }

          query = "select                                           \
                   usr.username,                                    \
                   shp.name,                                        \
                   dty.type,                                        \
                   tu.id_titik_ukur,                                \
                   dt.value,                                        \
                   dt.epochtime,                                    \
                   from_unixtime(dt.epochtime) as time_stamp        \
                  from                                              \
                   user as usr,                                     \
                   ship as shp,                                     \
                   titik_ukur as tu,                                \
                   data as dt,                                      \
                   data_type as dty                                 \
                  where usr.api_key = '"+api_key.toString()+"'      \
                    and usr.id_company = shp.id_company             \
                    and tu.id_ship = shp.id_ship                    \
                    and tu.id_data_type = dty.id_data_type          \
                    and dt.id_titik_ukur = tu.id_titik_ukur         \
                    and "+epochtime+"                               \
                    and shp.status = 1                              \
                    and shp.name like '%"+ship_name.toString()+"%'  \
                  order by shp.id_ship asc, dt.epochtime desc;";
          result = database.read(db,query,QStringList());
          if (result.length() > 0) {
              QJsonArray shipArray;
              QJsonObject shipObject;
              QJsonArray pointAray;
              QJsonObject pointObject;
              for (int i = 0; i < result.length(); i+=7) {
                  if (shipObject["NAME"].isUndefined() || shipObject["NAME"].isNull() ) {
                      shipObject["NAME"] = result.at(i+1);

                      pointObject["TYPE"] = result.at(i+2);
                      pointObject["POINT"] = result.at(i+3);
                      pointObject["VALUE"] = result.at(i+4);
                      pointObject["EPOCHTIME"] = result.at(i+5);
                      pointObject["TIMESTAMP"] = result.at(i+6);
                      pointAray.append(pointObject);
                  } else if (shipObject["NAME"].toString() != result.at(i+1)) {
                      shipObject["DATA"] = pointAray;
                      shipArray.append(shipObject);
                      for (int j = 0; j < pointAray.count(); j++) pointAray.removeAt(j);

                      shipObject["NAME"] = result.at(i+1);

                      pointObject["TYPE"] = result.at(i+2);
                      pointObject["POINT"] = result.at(i+3);
                      pointObject["VALUE"] = result.at(i+4);
                      pointObject["EPOCHTIME"] = result.at(i+5);
                      pointObject["TIMESTAMP"] = result.at(i+6);
                      pointAray.append(pointObject);
                  } else {
                      pointObject["TYPE"] = result.at(i+2);
                      pointObject["POINT"] = result.at(i+3);
                      pointObject["VALUE"] = result.at(i+4);
                      pointObject["EPOCHTIME"] = result.at(i+5);
                      pointObject["TIMESTAMP"] = result.at(i+6);
                      pointAray.append(pointObject);
                  }
              }
              shipObject["DATA"] = pointAray;
              shipArray.append(shipObject);
              json["SHIP"] = shipArray;
          } else {
              json["SHIP"] = "No Data Found!";
          }
      } else {
          json["ERROR"] = "API Key Not Found!";
      }
  } else {
      QStringList result = database.read(
                  db,
                  "select username from user where api_key is not null",
                  QStringList()
      );
      if (result.length() > 0) {
          json["USERNAME"] = QJsonArray::fromStringList(result);
      } else {
          json["ERROR"] = "No username has API Key!";
      }
  }
  json["PARAM"] = QJsonArray::fromStringList(parameter);
//  json["QUERY"] = query;
}
