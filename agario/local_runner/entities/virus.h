#ifndef VIRUS_H
#define VIRUS_H

#include "circle.h"
#include "ejection.h"


class Virus : public Circle
{
public:
    enum State { CREATED, HURT, EATER, MOVING, SPLIT };
    State logical;

protected:
    double speed;
    double angle, split_angle;

public:
    explicit Virus(int _id, double _x, double _y, double _radius, double _mass) :
        Circle(_id, _x, _y, _radius, _mass),
        logical(State::CREATED),
        speed(0.0),
        angle(0.0),
        split_angle(0.0)
    {}

    virtual ~Virus() {}

    virtual bool is_virus() const {
        return true;
    }

    void draw(QPainter &painter) const {
        painter.setPen(QPen(QBrush(Qt::black), 1));

        for (double angle = 0; angle < M_PI; angle += M_PI / 12) {
            double dx = qCos(angle) * radius;
            double dy = qSin(angle) * radius;
            painter.drawLine(x - dx, y - dy, x + dx, y + dy);
        }
    }

    double can_hurt(Circle *circle) const {
        if (circle->getR() < radius) {
            return INFINITY;
        }
        double qdist = circle->calc_qdist(x, y);
        double tR = radius * RAD_HURT_FACTOR + circle->getR();
        if (qdist < tR * tR) {
            return qdist;
        }
        return INFINITY;
    }

    double can_eat(Ejection *eject) {
        if (mass > eject->getM() * MASS_EAT_FACTOR) {
            double qdist = eject->calc_qdist(x, y);
            double tR = radius - eject->getR() * RAD_EAT_FACTOR;
            if (qdist < tR * tR) {
                return qdist;
            }
        }
        return INFINITY;
    }

    void eat(Ejection *eject) {
        mass += eject->getM();
        split_angle = eject->getA();
        logical = State::EATER;
    }

    bool can_split() {
        if (logical == State::CREATED || logical == State::MOVING || logical == State::EATER) {
            if (mass > Constants::instance().VIRUS_SPLIT_MASS) {
                return true;
            }
        }
        return false;
    }

    Virus *split_now(int new_id) {
        double new_speed = VIRUS_SPLIT_SPEED, new_angle = split_angle;

        Virus *new_virus = new Virus(new_id, x, y, Constants::instance().VIRUS_RADIUS, VIRUS_MASS);
        new_virus->set_impulse(new_speed, new_angle);

        mass = VIRUS_MASS;
        logical = State::CREATED;
        return new_virus;
    }

public:
    void set_impulse(double _speed, double _angle) {
        speed = qAbs(_speed);
        angle = _angle;
        logical = State::MOVING;
    }

    bool move(int max_x, int max_y) {
        double rB = x + radius, lB = x - radius;
        double dB = y + radius, uB = y - radius;

        double dx = speed * qCos(angle);
        double dy = speed * qSin(angle);

        bool changed = false;
        if (rB + dx < max_x && lB + dx > 0) {
            x += dx;
            changed = true;
        }
        if (dB + dy < max_y && uB + dy > 0) {
            y += dy;
            changed = true;
        }

        double visc = Constants::instance().VISCOSITY;
        if (speed > visc) {
            speed -= visc;
        }
        else {
            speed = 0.0;
            logical = State::CREATED;
        }
        return changed;
    }

    virtual QJsonObject toJson(bool mine=false) const {
        QJsonObject objData;
        objData.insert("X", QJsonValue(x));
        objData.insert("Y", QJsonValue(y));
        objData.insert("M", QJsonValue(mass));
        objData.insert("T", QJsonValue("V"));
        objData.insert("Id", QJsonValue(QString::number(id)));
        return objData;
    }
};

typedef QVector<Virus*> VirusArray;

#endif // VIRUS_H
