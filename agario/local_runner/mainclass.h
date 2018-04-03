#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "strategymodal.h"
#include "mechanic.h"

class MainClass
{

private:
    Mechanic *mechanic;
    StrategyModal *sm;
    long T;
    bool isRun;

public:
    explicit MainClass() :
        mechanic(new Mechanic),
        sm(new StrategyModal)
    {
        clear_game();
        init_game();
    }

    ~MainClass()
    {
        if (mechanic) delete mechanic;
        if (sm) delete sm;
    }
    bool isGameRun() { return isRun; }

    void init_game()
    {
        time(&T);
        mechanic->init_objects(T, [this] (Player *player) {
            int pId = player->getId();
            player->set_color(sm->get_color(pId));

            Strategy *strategy = sm->get_strategy(pId);
            return strategy;
        });
    }
    void clear_game() { mechanic->clear_objects(false); }

    void play_game()
    {
        int tick;
        isRun = true;
        while (isRun)
        {
            qDebug() << tick;
            tick = mechanic->tickEvent();
            if (tick >= Constants::instance().GAME_TICKS) isRun = false;
        }
        qDebug("Game scores");
        QMap<int, int> scores = mechanic->get_scores();
        for (int player_id : scores.keys()) { qDebug() << player_id << scores[player_id]; }
        clear_game();
    }
};

#endif // MAINWINDOW_H
