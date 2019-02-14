#ifndef API_MARINE_H
#define API_MARINE_H

#include <action.h>

class API_Marine : public qttp::Action
{
  public:

    API_Marine() : Action()
    {
    }

    const char* getName() const
    {
      return "api-marine-action";
    }

    void onGet(qttp::HttpData& data)
    {
      static const QHash<QString, QString> info = {{"iMac", "apple"}, {"inspiron", "dell"}, {"z800", "hp"}};
      QJsonValue type = data.getRequest().getJson()["api_key"];
      if(type.isString() && info.contains(type.toString()))
      {
        data.getResponse().getJson()["maker"] = info[type.toString()];
        return;
      }
      qttp::HttpResponse& response = data.getResponse();
      QJsonObject& json = response.getJson();
      json["list"] = QJsonArray::fromStringList(desktops);
    }
};

#endif // API_MARINE_H
