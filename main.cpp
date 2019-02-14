#include <QCoreApplication>
#include "web_server.h"
#include "worker.h"

#define VERSION "Beta Test"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc == 2) {
        QString arg1(argv[1]);
        if (arg1 == QLatin1String("-v") ||
            arg1 == QLatin1String("--version")) {
            QString compilationTime = QString("%1 %2").arg(__DATE__).arg(__TIME__);
            QString version = VERSION;
            printf("Marine Service Version:  %s\nPT. DaunBiru Engineering\nwww.daunbiru.com\n\n"
                   "build on: %s (UTC+7)\n",
                   version.toUtf8().data(),
                   compilationTime.toUtf8().data());
        }
        return 0;
    } else {
        printf(":: Marine Service Started ::\n");

        web_server ws;
        QThread Thread_ws;
        ws.doSetup(Thread_ws);
        ws.moveToThread(&Thread_ws);
        Thread_ws.start();

        worker w;
        return a.exec();
    }
}
