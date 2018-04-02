#pragma once

#include <QGraphicsScene>

class Mechanic;
class QPointF;
class QKeyPressEvent;
class QGraphicsSceneMouseEvent;
class QKeyEvent;
class QGraphicsSceneMouseEvent;

class GameScene : public QGraphicsScene {
    Q_OBJECT

public:
    using QGraphicsScene::QGraphicsScene;

    virtual ~GameScene();

    Mechanic* mechanic();
    void setMechanic(Mechanic* mechanic);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void onMouse(QGraphicsSceneMouseEvent* event);

private:
    Mechanic* m_mechanic = nullptr;
    bool m_isMousePressed = false;
};
