QT       += core

CONFIG += c++11 warn_off
TARGET = local_runner_console
TEMPLATE = app

SOURCES += main.cpp

HEADERS  += \
    mechanic.h \
    logger.h \
    entities/food.h \
    entities/circle.h \
    constants.h \
    entities/virus.h \
    entities/player.h \
    strategies/strategy.h \
    entities/ejection.h \
    strategymodal.h \
    strategies/custom.h \
    mainclass.h