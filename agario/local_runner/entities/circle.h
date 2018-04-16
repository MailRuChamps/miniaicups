#ifndef CIRCLE_H
#define CIRCLE_H

#include "constants.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QPainter>
#include <QPoint>


struct Direct
{
    explicit Direct(double _x, double _y) : x(_x), y(_y), split(false), eject(false) {}

public:
    double x, y;
    bool split;
    bool eject;
    
    static double sqr(double x) {return x * x;}
    void limit() {
        double w = Constants::instance().GAME_WIDTH;
        double h = Constants::instance().GAME_WIDTH;
        double mcx = w * 0.5;
        double mcy = h * 0.5;
        double rvx = x - mcx;
        double rvy = y - mcy;
        double rv_len = qSqrt(sqr(rvx) + sqr(rvy));
        double dist_from_map_center_to_direct_pos = qSqrt(sqr(w) + sqr(h)) * 1.25;
        if (rv_len<dist_from_map_center_to_direct_pos || !rv_len) return;
        double k = dist_from_map_center_to_direct_pos / rv_len;
        x = mcx + rvx * k;
        y = mcy + rvy * k;
    }
};


class Circle
{
protected:
    int id;
    double x, y;
    double radius;
    double mass;

public:
    explicit Circle(int _id, double _x, double _y, double _radius, double _mass) :
        id(_id),
        x(_x),
        y(_y),
        radius(_radius),
        mass(_mass)
    {}

    virtual ~Circle() {}

public:
    int getId() const {
        return id;
    }

    QString id_to_str() const {
        return QString::number(id);
    }

    double getX() const {
        return x;
    }

    double getY() const {
        return y;
    }

    double getR() const {
        return radius;
    }

    double getM() const {
        return mass;
    }

    virtual bool is_my_eject(Circle *player) const {
        return false;
    }

    virtual bool is_food() const {
        return false;
    }

    virtual bool is_virus() const {
        return false;
    }

    virtual bool is_player() const {
        return false;
    }

public:
    double calc_dist(double from_x, double from_y) const {
        double dx = x - from_x, dy = y - from_y;
        return qSqrt(dx * dx + dy * dy);
    }

    double calc_qdist(double from_x, double from_y) const {
        double dx = x - from_x, dy = y - from_y;
        return dx * dx + dy * dy;
    }

    bool is_intersected(Circle *circle) const {
        double qdist = circle->calc_qdist(x, y);
        double dR = radius + circle->radius;
        return qdist < dR * dR;
    }

    bool is_intersected(double _x, double _y, double _radius) const {
        double qdist = calc_qdist(_x, _y);
        double dR = radius + _radius;
        return qdist < dR * dR;
    }

    virtual QJsonObject toJson(bool mine=false) const = 0;
};

typedef QVector<Circle*> CircleArray;


#endif // CIRCLE_H
