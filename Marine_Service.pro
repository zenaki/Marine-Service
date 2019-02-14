QT += core sql network
QT -= gui

CONFIG += c++11

TARGET = marine-service
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

SOURCES += main.cpp \
    worker.cpp \
    config.cpp \
    mysql.cpp \
    api.cpp \
    api_marine.cpp \
    web_server.cpp

HEADERS += \
    worker.h \
    config.h \
    mysql.h \
#    api_marine.h \
#    phones.h \
#    desktops.h \
#    tablets.h \
#    laptops.h \
    utama.h \
    api.h \
    api_marine.h \
    web_server.h


include($$PWD/config/config.pri)
include($$PWD/../../QttpServer/core.pri)

MOC_DIR = $$PWD/build/$$QTBUILDTYPE
RCC_DIR = $$PWD/build/$$QTBUILDTYPE
UI_DIR = $$PWD/build/$$QTBUILDTYPE
OBJECTS_DIR = $$PWD/build/$$QTBUILDTYPE
DESTDIR = $$PWD/build/$$QTBUILDTYPE
