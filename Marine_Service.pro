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
    mysql.cpp

HEADERS += \
    worker.h \
    config.h \
    mysql.h \
    utama.h
