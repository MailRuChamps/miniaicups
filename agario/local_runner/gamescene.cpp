#include "gamescene.h"

#include "mechanic.h"
#include <QGraphicsSceneMouseEvent>

GameScene::~GameScene() {
}

Mechanic* GameScene::mechanic() {
    return m_mechanic;
}

void GameScene::setMechanic(Mechanic* mechanic) {
    if (m_mechanic) {
        m_mechanic->setScene(nullptr);
    }
    m_mechanic = mechanic;
    m_mechanic->setScene(this);
}

void GameScene::keyPressEvent(QKeyEvent* event) {
    return m_mechanic->keyPressEvent(event);
}

void GameScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }
    m_isMousePressed = true;
    return onMouse(event);
}

void GameScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }
    m_isMousePressed = false;
}

void GameScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    return onMouse(event);
}

void GameScene::onMouse(QGraphicsSceneMouseEvent* event) {
    if (!m_isMousePressed) {
        return;
    }
    return m_mechanic->onMouse(event->scenePos());
}
