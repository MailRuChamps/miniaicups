#ifndef EJECTION_H
#define EJECTION_H

#include "circle.h"


class Ejection : public Circle
{
protected:
    int color;
    int player;
    double speed;
    double angle;

public:
    explicit Ejection(int _id, double _x, double _y, double _radius, double _mass, int player) :
        Circle(_id, _x, _y, _radius, _mass),
        speed(0.0),
        angle(0.0),
        player(player)
    {
        color = rand() % 14 + 4;
    }

    virtual ~Ejection() {}

    int getC() const {
        return color;
    }

    double getA() const {
        return angle;
    }

    virtual bool is_my_eject(Circle *player) const {
        return this->player == player->getId();
    }

    virtual bool is_food() const {
        return true;
    }

    void draw(QPainter &painter) const {
        painter.setPen(QPen(QBrush(Qt::black), 1));
        painter.setBrush(Qt::GlobalColor(color));

        int ix = int(x), iy = int(y), ir = int(radius);
        painter.drawEllipse(QPoint(ix, iy), ir, ir);
    }

public:
    void set_impulse(double _speed, double _angle) {
        speed = qAbs(_speed);
        angle = _angle;
    }

    bool move(int max_x, int max_y) {
        if (speed == 0.0) {
            return false;
        }
        double dx = speed * qCos(angle);
        double dy = speed * qSin(angle);

        double new_x = qMax(radius, qMin(max_x - radius, x + dx));
        bool changed = (x != new_x);
        x = new_x;

        double new_y = qMax(radius, qMin(max_y - radius, y + dy));
        changed |= (y != new_y);
        y = new_y;

        speed = qMax(0.0, speed - Constants::instance().VISCOSITY);
        return changed;
    }

    int get_player() {
        return player;
    }

    double get_speed() {
        return speed;
    }

    double get_angle() {
        return angle;
    }

public:
    virtual QJsonObject toJson(bool mine=false) const {
        QJsonObject objData;
        objData.insert("X", QJsonValue(x));
        objData.insert("Y", QJsonValue(y));
        objData.insert("T", QJsonValue("E"));
        objData.insert("Id", QJsonValue(QString::number(id)));
        objData.insert("pId", QJsonValue(player));
        return objData;
    }
};

typedef QVector<Ejection*> EjectionArray;

#endif // EJECTION_H
