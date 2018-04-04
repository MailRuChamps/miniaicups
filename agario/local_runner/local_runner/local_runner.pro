#-------------------------------------------------
#
# Project created by QtCreator 2018-02-13T16:55:01
#
#-------------------------------------------------

DEFINES += LOCAL_RUNNER

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 warn_off
TARGET = local_runner
TEMPLATE = app


SOURCES += local_runner.cpp

HEADERS  += mainwindow.h \
    mechanic.h \
    logger.h \
    entities/food.h \
    entities/circle.h \
    constants.h \
    entities/virus.h \
    entities/player.h \
    strategies/strategy.h \
    strategies/bymouse.h \
    entities/ejection.h \
    strategymodal.h \
    strategies/custom.h

FORMS    += mainwindow.ui \
    strategymodal.ui

LIBS += -lz
