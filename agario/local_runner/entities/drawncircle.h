#pragma once

#include "circle.h"

class QGraphicsScene;

class DrawnCircle : public Circle {
public:
    using Circle::Circle;
    virtual ~DrawnCircle();

    void setScene(QGraphicsScene* scene);

protected:
    virtual void updateItems() = 0;
    virtual void addItemsToScene() = 0;
    virtual void removeItemsFromScene() = 0;

protected:
    QGraphicsScene* m_scene = nullptr;
};
