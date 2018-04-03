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
        motion(id % 4)
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

        int waypoints[4][2] = {
            {110, 110},
            {550, 110},
            {550, 550},
            {110, 550}
        };

        if (mine->calc_dist(waypoints[motion][0], waypoints[motion][1]) < 5) {
            motion = (motion + 1) % 4;
        }

        return Direct(waypoints[motion][0], waypoints[motion][1]);
    }
};

typedef QVector<Strategy*> StrategyArray;

#endif // STARTEGY_H
