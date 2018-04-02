#ifndef EJECTION_H
#define EJECTION_H

#include "circle.h"

#include <QScopedPointer>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>

class Ejection : public DrawnCircle
{
protected:
    int color;
    int player;
    double speed;
    double angle;

public:
    explicit Ejection(int _id, double _x, double _y, double _radius, double _mass, int player) :
        DrawnCircle(_id, _x, _y, _radius, _mass),
        speed(0.0),
        angle(0.0),
        player(player)
    {
        color = rand() % 14 + 4;

        m_item.reset(new QGraphicsEllipseItem());
        m_item->setPen(QPen(QBrush(Qt::black), 1));
        m_item->setBrush(Qt::GlobalColor(color));

        updateItems();
    }

    virtual void addItemsToScene() override {
        m_scene->addItem(m_item.data());
    }

    void removeItemsFromScene() override {
        m_scene->removeItem(m_item.data());
    }

    virtual ~Ejection() {
        removeItemsFromScene();
    }

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

    virtual void updateItems() override {
        m_item->setRect(x - radius, y - radius, 2 * radius, 2 * radius);
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

        if (changed) {
            updateItems();
        }

        return changed;
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

private:
    QScopedPointer<QGraphicsEllipseItem> m_item;
};

typedef QVector<Ejection*> EjectionArray;

#endif // EJECTION_H
