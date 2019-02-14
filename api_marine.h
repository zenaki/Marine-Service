#ifndef API_MARINE_H
#define API_MARINE_H

#include <action.h>
#include "mysql.h"

class api_marine : public qttp::Action
{
public:
    api_marine();

    mysql database;
    QSqlDatabase db;

    QStringList marines;
    const char* getName() const
    {
      return "api-marine";
    }
    void onGet(qttp::HttpData& data);
};

#endif // API_MARINE_H
