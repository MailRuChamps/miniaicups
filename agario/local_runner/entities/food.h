#ifndef FOOD_H
#define FOOD_H

#include "circle.h"


class Food : public Circle
{
protected:
    int color;

public:
    explicit Food(int _id, double _x, double _y, double _radius, double _mass) :
        Circle(_id, _x, _y, _radius, _mass)
    {
        color = rand() % 14 + 4;
    }

    virtual ~Food() {}

    int getC() const {
        return color;
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
    virtual QJsonObject toJson(bool mine=false) const {
        QJsonObject objData;
        objData.insert("X", QJsonValue(x));
        objData.insert("Y", QJsonValue(y));
        objData.insert("T", QJsonValue("F"));
        return objData;
    }
};

typedef QVector<Food*> FoodArray;


#endif // FOOD_H
