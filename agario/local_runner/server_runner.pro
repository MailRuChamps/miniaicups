DEFINES += SERVER_RUNNER

QT += core network gui

CONFIG += c++11 warn_off

TARGET = server_runner
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS  += mechanic.h \
    logger.h \
    entities/food.h \
    entities/circle.h \
    constants.h \
    strategies/strategy.h \
    strategies/bymouse.h \
    entities/virus.h \
    entities/player.h \
    entities/ejection.h \
    tcp_server.h \
    tcp_connect.h

SOURCES += server_runner.cpp

DISTFILES += \
    ../Dockerfile

LIBS += -lz
