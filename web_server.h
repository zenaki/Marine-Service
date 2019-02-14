#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <QObject>
#include <qttpserver>
//#include <desktops.h>
//#include <phones.h>
//#include <tablets.h>
//#include <laptops.h>

#include "api_marine.h"

class web_server : public QObject
{
    Q_OBJECT
public:
    explicit web_server(QObject *parent = 0);

    void doSetup(QThread &cThread);

private slots:
    void doWork();
};

#endif // WEB_SERVER_H
