#ifndef CONSOLE_RUNNER_H
#define CONSOLE_RUNNER_H

#include <QCoreApplication>
#include <qobject.h>
#include <strategies/custom.h>
#include "mechanic.h"


class ConsoleRunner : public QObject {
    Q_OBJECT

private:
    QStringList strategies;
    Mechanic *mechanic;

public:
    explicit ConsoleRunner(QStringList _strategies) :
        strategies(_strategies),
        mechanic(new Mechanic())
    {}
    ~ConsoleRunner() {
        delete mechanic;
    }
    void run() {
        QMap<int, Strategy*> strategies_map;
        for (int i = 1; i <= strategies.length(); i++) {
            strategies_map.insert(i, new Custom(i, strategies[i - 1]));
        }
        std::string seed = Constants::instance().generate_seed();
        mechanic->init_objects(seed, [this, strategies_map] (Player *player) {
            int pId = player->getId();

            Strategy *strategy = strategies_map.value(pId, new Strategy(pId));
            Custom *custom = dynamic_cast<Custom*>(strategy);
            if (custom != NULL) {
                connect(custom, SIGNAL(error(QString)), this, SLOT(on_error(QString)));
            }
            return strategy;
        });
        qDebug() << "start";

        int tick = 0;
        while(tick < Constants::instance().GAME_TICKS && !mechanic->known()) {
            tick = mechanic->tickEvent();
            if (tick % 100 == 0) qDebug() << tick;
        }

        for (Strategy *strategy : strategies_map.values()) {
            Custom *custom = dynamic_cast<Custom*>(strategy);
            if (custom != NULL) custom->get_looger()->flush(false);

        }
        mechanic->get_logger()->flush(false);
        qDebug() << "done";
    }
public slots:
    void on_error(QString msg) {
        qDebug() << msg;
    }
};

#endif // CONSOLE_RUNNER_H
