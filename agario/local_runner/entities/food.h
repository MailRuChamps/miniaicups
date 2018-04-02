#ifndef FOOD_H
#define FOOD_H

#include "drawncircle.h"

#include <QScopedPointer>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>

class Food : public DrawnCircle
{
protected:
    int color;

public:
    explicit Food(int _id, double _x, double _y, double _radius, double _mass) :
        DrawnCircle(_id, _x, _y, _radius, _mass)
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

    virtual ~Food() {
        removeItemsFromScene();
    }

    int getC() const {
        return color;
    }

    virtual bool is_food() const {
        return true;
    }

    virtual void updateItems() override {
        m_item->setRect(x - radius, y - radius, 2 * radius, 2 * radius);
    }

public:
    virtual QJsonObject toJson(bool mine=false) const {
        QJsonObject objData;
        objData.insert("X", QJsonValue(x));
        objData.insert("Y", QJsonValue(y));
        objData.insert("T", QJsonValue("F"));
        return objData;
    }

private:
    QScopedPointer<QGraphicsEllipseItem> m_item;
};

typedef QVector<Food*> FoodArray;


#endif // FOOD_H
