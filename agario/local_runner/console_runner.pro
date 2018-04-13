DEFINES += CONSOLE_RUNNER

QT += core network
QT -= gui

CONFIG += c++11 console warn_off
CONFIG -= app_bundle

TEMPLATE = app

TARGET = console_runner

SOURCES += \
        console_runner.cpp

LIBS += -lz


HEADERS  += mechanic.h \
    logger.h \
    entities/food.h \
    entities/circle.h \
    constants.h \
    strategies/strategy.h \
    strategies/custom.h \
    entities/virus.h \
    entities/player.h \
    entities/ejection.h \
    console_runner.h
