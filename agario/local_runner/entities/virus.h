#ifndef VIRUS_H
#define VIRUS_H

#include "circle.h"
#include "ejection.h"

#include <QGraphicsLineItem>

#include <memory>
#include <vector>

class Virus : public DrawnCircle
{
protected:
    double speed;
    double angle, split_angle;

public:
    explicit Virus(int _id, double _x, double _y, double _radius, double _mass) :
        DrawnCircle(_id, _x, _y, _radius, _mass),
        speed(0.0),
        angle(0.0),
        split_angle(0.0)
    {
        m_items.resize(12);
        for (int i = 0; i < 12; ++i) {
            m_items[i].reset(new QGraphicsLineItem());
            m_items[i]->setPen(QPen(QBrush(Qt::black), 1));
        }
        updateItems();
    }

    virtual void addItemsToScene() override {
        for (auto& item: m_items) {
            m_scene->addItem(item.get());
        }
    }

    void removeItemsFromScene() override {
        for (auto& item: m_items) {
            m_scene->removeItem(item.get());
        }
    }

    virtual ~Virus() {
        removeItemsFromScene();
    }

    virtual bool is_virus() const {
        return true;
    }

    virtual void updateItems() override {
        QPointF center(x, y);
        for (int i = 0; i < 12; ++i) {
            double angle = M_PI * i / 12;
            QPointF delta = QPointF(qCos(angle), qSin(angle)) * radius;
            m_items[i]->setLine(QLineF(center - delta, center + delta));
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
        if (mass > eject->getM() * MASS_EAT_FACTOR) { // eat ejection
            double dist = eject->calc_dist(x, y);
            if (dist - eject->getR() + (eject->getR() * 2) * DIAM_EAT_FACTOR < radius) {
                return radius - dist;
            }
        }
        return -INFINITY;
    }

    void eat(Ejection *eject) {
        mass += eject->getM();
        split_angle = eject->getA();
    }

    bool can_split() {
        return mass > Constants::instance().VIRUS_SPLIT_MASS;
    }

    Virus *split_now(int new_id) {
        double new_speed = VIRUS_SPLIT_SPEED, new_angle = split_angle;

        Virus *new_virus = new Virus(new_id, x, y, Constants::instance().VIRUS_RADIUS, VIRUS_MASS);
        new_virus->set_impulse(new_speed, new_angle);

        mass = VIRUS_MASS;
        return new_virus;
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

        speed = std::max(0.0, speed - Constants::instance().VISCOSITY);
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

private:
    std::vector<std::unique_ptr<QGraphicsLineItem>> m_items;
};

typedef QVector<Virus*> VirusArray;

#endif // VIRUS_H
