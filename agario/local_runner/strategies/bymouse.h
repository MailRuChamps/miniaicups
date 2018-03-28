#ifndef BYMOUSE_H
#define BYMOUSE_H

#include "strategy.h"

#include <QMouseEvent>
#include <QKeyEvent>


class ByMouse : public Strategy
{
protected:
    int x, y;
    bool eject;
    bool split;

public:
    explicit ByMouse(int _id) :
        Strategy(_id)
    {
        Constants &ins = Constants::instance();
        x = ins.GAME_WIDTH / 2; y = ins.GAME_HEIGHT / 2;
    }

    virtual ~ByMouse() {}

    virtual Direct tickEvent(const PlayerArray &fragments, const CircleArray &objects) {
        Direct direct = Direct(x, y);
        if (eject) {
            direct.eject = true;
            eject = false;
        }
        if (split) {
            direct.split = true;
            split = false;
        }
        return direct;
    }

    void set_mouse(int _x, int _y) {
        x = _x; y = _y;
    }

    void set_key(QKeyEvent *event) {
        int key = event->key();
        if (key == Qt::Key_W) {
            eject = true;
        }
        if (key == Qt::Key_Space) {
            split = true;
        }
    }
};

#endif // BYMOUSE_H
