QT += core network
QT -= gui

CONFIG += c++11 warn_off

TARGET = client
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

DISTFILES += \
    ../Dockerfile \
    run.sh

HEADERS += \
    tcp_client.h \
    constants.h
