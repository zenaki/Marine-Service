#include "web_server.h"

web_server::web_server(QObject *parent) : QObject(parent)
{

}

void web_server::doSetup(QThread &cThread)
{
    connect(&cThread,SIGNAL(started()),this,SLOT(doWork()));
}

void web_server::doWork()
{
    // Always initialize in the main thread.
    qttp::HttpServer* httpSvr = qttp::HttpServer::getInstance();
    // httpSvr->initialize();
    httpSvr->initRoutes(QString(PATH_MONITA) + "/config/routes.json");
    httpSvr->initGlobal(QString(PATH_MONITA) + "/config/global.json");

    auto action = httpSvr->addAction<api_marine>();
    action->registerRoute(qttp::HttpMethod::GET, "marine");
    action->registerRoute(qttp::HttpMethod::GET, "marine/:api_key/all/");
    action->registerRoute(qttp::HttpMethod::GET, "marine/:api_key/:ship_name/");
    action->registerRoute(qttp::HttpMethod::GET, "marine/:api_key/:ship_name/:start_date");
    action->registerRoute(qttp::HttpMethod::GET, "marine/:api_key/:ship_name/:start_date/:end_date");
    action->registerRoute(qttp::HttpMethod::GET, "marine/:api_key/all/dd-MM-yyyy HH:mm:ss/dd-MM-yyyy HH:mm:ss");
    action->registerRoute(qttp::HttpMethod::GET, "marine/fdi3Mk7iEXSqk/all/2018-10-01 00:00:00/2018-10-01 01:00:00");

    qttp::HttpServer::ServerInfo info = httpSvr->getServerInfo();
    info.title = "Monita Marine API";
    info.description = "API for collecting raw data Monita Marine";
    info.companyName = "PT. Daun Biru Engineering";
    httpSvr->setServerInfo(info);

    httpSvr->initHttpDirectory(QDir().absoluteFilePath(QString(PATH_MONITA) + "/www"));
    httpSvr->initSwagger(true);

    // Libuv runs in its own thread.
    httpSvr->startServer();
}
