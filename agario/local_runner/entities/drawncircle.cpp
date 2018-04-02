#include "drawncircle.h"

DrawnCircle::~DrawnCircle() {
}

void DrawnCircle::setScene(QGraphicsScene* scene) {
    if (m_scene) {
        removeItemsFromScene();
    }
    m_scene = scene;
    if (m_scene) {
        addItemsToScene();
    }
}
