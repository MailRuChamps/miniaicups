#ifndef STARTEGY_H
#define STARTEGY_H

#include "../entities/player.h"


class Strategy : public QObject
{
protected:
    int id;
    int motion;

public:
    explicit Strategy(int _id) :
        id(_id),
        motion(0)
    {}

    virtual ~Strategy() {}

    int getId() const { return id; }

    virtual Direct tickEvent(const PlayerArray &fragments, const CircleArray &objects) {
        double min_dist = INFINITY;
        Circle *goal = NULL;
        Circle *mine = fragments[0];

        for (Circle *obj : objects) {
            if (obj->is_food()) {
                double dist = obj->calc_qdist(mine->getX(), mine->getY());
                if (dist < min_dist) {
                    min_dist = dist;
                    goal = obj;
                }
            }
        }
        if (goal != NULL) {
            Direct direct(goal->getX(), goal->getY());
            if (rand() % 1000 < 5) {
                direct.split = true;
            }
            return direct;
        }
        if (motion == 0) {
            if (mine->getX() == 110 && mine->getY() == 110) { motion = 1; }
            return Direct(110, 110);
        }
        else if (motion == 1) {
            if (mine->getX() == 550 && mine->getY() == 110) { motion = 2; }
            return Direct(550, 110);
        }
        else if (motion == 2) {
            if (mine->getX() == 550 && mine->getY() == 550) { motion = 3; }
            return Direct(550, 550);
        }
        else if (motion == 3) {
            if (mine->getX() == 110 && mine->getY() == 550) { motion = 0; }
            return Direct(110, 550);
        }
        return Direct(330, 330);
    }
};

typedef QVector<Strategy*> StrategyArray;

#endif // STARTEGY_H
