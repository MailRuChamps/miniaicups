#ifndef PLAYER_H
#define PLAYER_H

#include "drawncircle.h"
#include "ejection.h"

#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QScopedPointer>

class Player : public DrawnCircle
{
public:
    bool is_fast;
    int fuse_timer;

protected:
    double speed, angle;
    int fragmentId;
    int color;
    int score;
    double vision_radius;
    double cmd_x, cmd_y;

public:
    explicit Player(int _id, double _x, double _y, double _radius, double _mass, const int fId=0) :
        DrawnCircle(_id, _x, _y, _radius, _mass),
        is_fast(false),
        speed(0), angle(0),
        fragmentId(fId),
        score(0),
        vision_radius(0),
        cmd_x(0), cmd_y(0)
    {
        color = _id + 7;
        if (fId > 0) {
            fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        }

        m_item.reset(new QGraphicsEllipseItem());
        m_textItem.reset(new QGraphicsTextItem());
        m_velocityLine.reset(new QGraphicsLineItem());
        m_commandLine.reset(new QGraphicsLineItem());
        m_visionLine.reset(new QGraphicsEllipseItem());

        m_item->setPen(QPen(QBrush(Qt::black), 1));
        m_item->setBrush(Qt::GlobalColor(color));
        m_textItem->setDefaultTextColor(Qt::black);
        m_velocityLine->setPen(QPen(QBrush(Qt::green), 1));
        m_commandLine->setPen(QPen(QBrush(Qt::red), 1));
        m_visionLine->setPen(QPen(QBrush(Qt::black), 1, Qt::DashLine));

        m_velocityLine->hide();
        m_commandLine->hide();
        m_visionLine->hide();

        updateItems();
    }

    virtual void addItemsToScene() override {
        m_scene->addItem(m_item.data());
        m_scene->addItem(m_textItem.data());
        m_scene->addItem(m_velocityLine.data());
        m_scene->addItem(m_commandLine.data());
        m_scene->addItem(m_visionLine.data());
    }

    void removeItemsFromScene() override {
        m_scene->removeItem(m_item.data());
        m_scene->removeItem(m_textItem.data());
        m_scene->removeItem(m_velocityLine.data());
        m_scene->removeItem(m_commandLine.data());
        m_scene->removeItem(m_visionLine.data());
    }

    QGraphicsItem* velocityLine() {
        return m_velocityLine.data();
    }

    const QGraphicsItem* velocityLine() const {
        return m_velocityLine.data();
    }

    QGraphicsItem* commandLine() {
        return m_commandLine.data();
    }

    const QGraphicsItem* commandLine() const {
        return m_commandLine.data();
    }

    QGraphicsItem* visionLine() {
        return m_visionLine.data();
    }

    const QGraphicsItem* visionLine() const {
        return m_visionLine.data();
    }

    virtual ~Player() {
        removeItemsFromScene();
    }

    QString id_to_str() const {
        if (fragmentId > 0) {
            return QString::number(id) + "." + QString::number(fragmentId);
        }
        return QString::number(id);
    }

    int get_fId() const {
        return fragmentId;
    }

    int getC() const {
        return color;
    }

    double getA() const {
        return angle;
    }

    void set_color(int _color) {
        color = _color;
        m_item->setBrush(Qt::GlobalColor(color));
    }

    virtual bool is_player() const {
        return true;
    }

    int get_score() {
        int _score = score;
        score = 0;
        return _score;
    }

    QPointF get_direct_norm() const {
        double dx = cmd_x - x, dy = cmd_y - y;
        double dist = qSqrt(dx * dx + dy * dy);
        if (dist > 0) {
            double factor = 50 / dist;
            return QPointF(x + dx * factor, y + dy * factor);
        }
        return QPointF(x, y);
    }

    double getVR() const {
        return vision_radius;
    }

public:
    void set_impulse(double new_speed, double new_angle) {
        speed = qAbs(new_speed);
        angle = new_angle;
        is_fast = true;
        updateItems();
    }

    void apply_viscosity(double usual_speed) {
        if (is_fast && speed > usual_speed) {
            speed = std::max(speed - Constants::instance().VISCOSITY, usual_speed);
        }
        if (is_fast && speed <= usual_speed) {
            is_fast = false;
            speed = usual_speed;
        }
        updateItems();
    }

    virtual void updateItems() override {
        QPointF center(x, y);

        m_item->setRect(x - radius, y - radius, 2 * radius, 2 * radius);

        m_textItem->setPlainText(QString::number(mass));
        QRectF textRect = m_textItem->boundingRect();
        QPointF textVec = textRect.bottomRight() - textRect.topLeft();
        m_textItem->setPos(center - textVec / 2);

        QPointF speedVec = QPointF(qCos(angle), qSin(angle)) * speed * DRAW_SPEED_FACTOR;
        m_velocityLine->setLine(QLineF(center, center + speedVec));

        QPointF norm = get_direct_norm();
        m_commandLine->setLine(QLineF(center, norm));

        int ix = int(x), iy = int(y), ir = int(radius);
        int ivr = int(vision_radius);

        QPointF shifted = center + QPointF(qCos(angle), qSin(angle)) * VIS_SHIFT;
        QPointF vision(vision_radius, vision_radius);
        m_visionLine->setRect(QRectF(shifted - vision, shifted + vision));
    }

    bool update_vision(int frag_cnt) {
        double new_vision;
        if (frag_cnt == 1) {
            new_vision = radius * VIS_FACTOR;
        }
        else {
            new_vision = radius * VIS_FACTOR_FR * qSqrt(frag_cnt);
        }
        if (vision_radius != new_vision) {
            vision_radius = new_vision;
            updateItems();
            return true;
        }
        return false;
    }

    bool can_see(const Circle *circle) const {
        double xVisionCenter = x + qCos(angle) * VIS_SHIFT;
        double yVisionCenter = y + qSin(angle) * VIS_SHIFT;
        double qdist = circle->calc_qdist(xVisionCenter, yVisionCenter);

        return (qdist < (vision_radius + circle->getR()) * (vision_radius + circle->getR()));
    }

    double can_eat(Circle *food) const {
        if (food->is_player() && food->getId() == id) {
            return -INFINITY;
        }
        if (mass > food->getM() * MASS_EAT_FACTOR) { // eat anything
            double dist = food->calc_dist(x, y);
            if (dist - food->getR() + (food->getR() * 2) * DIAM_EAT_FACTOR < radius) {
                return radius - dist;
            }
        }
        return -INFINITY;
    }

    void eat(Circle *food, bool is_last=false) {
        mass += food->getM();
        updateItems();

        if (food->is_my_eject(this)) {
            return;
        } else if (food->is_food()) {
            score += SCORE_FOR_FOOD;
        }
        else if (food->is_player()) {
            score += (! is_last)? SCORE_FOR_PLAYER : SCORE_FOR_LAST;
        }
    }

    bool can_burst(int yet_cnt) const {
        if (mass < MIN_BURST_MASS * 2) {
            return false;
        }
        int frags_cnt = int(mass / MIN_BURST_MASS);
        if (frags_cnt > 1 && yet_cnt + 1 <= Constants::instance().MAX_FRAGS_CNT) {
            return true;
        }
        return false;
    }

    void burst_on(Circle *virus) {
        double dist = calc_dist(virus->getX(), virus->getY());
        double dy = y - virus->getY(), dx = x - virus->getX();
        double new_angle = 0.0;

        if (dist > 0) {
            new_angle = qAsin(dy / dist);
            if (dx < 0) {
                new_angle = M_PI - new_angle;
            }
        }
        angle = new_angle;
        double max_speed = Constants::instance().SPEED_FACTOR / qSqrt(mass);
        if (speed < max_speed) {
            speed = max_speed;
        }
        mass += BURST_BONUS;
        score += SCORE_FOR_BURST;
        updateItems();
    }

    void copyPropertiesTo(Player* other) const {
        other->set_color(color);
        other->velocityLine()->setVisible(velocityLine()->isVisible());
        other->commandLine()->setVisible(commandLine()->isVisible());
        other->visionLine()->setVisible(visionLine()->isVisible());
        other->setScene(m_scene);
    }

    QVector<Player*> burst_now(int max_fId, int yet_cnt) {
        QVector<Player*> fragments;
        int new_frags_cnt = int(mass / MIN_BURST_MASS) - 1;
        int max_cnt = Constants::instance().MAX_FRAGS_CNT - yet_cnt;
        if (new_frags_cnt > max_cnt) {
            new_frags_cnt = max_cnt;
        }

        double new_mass = mass / (new_frags_cnt + 1);
        double new_radius = Constants::instance().RADIUS_FACTOR * qSqrt(new_mass);

        for (int I = 0; I < new_frags_cnt; I++) {
            int new_fId = max_fId + I + 1;
            Player *new_fragment = new Player(id, x, y, new_radius, new_mass, new_fId);
            copyPropertiesTo(new_fragment);
            fragments.append(new_fragment);

            double burst_angle = angle - BURST_ANGLE_SPECTRUM / 2 + I * BURST_ANGLE_SPECTRUM / new_frags_cnt;
            new_fragment->set_impulse(BURST_START_SPEED, burst_angle);
        }
        set_impulse(BURST_START_SPEED, angle + BURST_ANGLE_SPECTRUM / 2);

        fragmentId = max_fId + new_frags_cnt + 1;
        mass = new_mass;
        radius = new_radius;
        fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        updateItems();
        return fragments;
    }

    bool can_split(int yet_cnt) const {
        if (yet_cnt + 1 <= Constants::instance().MAX_FRAGS_CNT) {
            if (mass > MIN_SPLIT_MASS) {
                return true;
            }
        }
        return false;
    }

    Player *split_now(int max_fId) {
        double new_mass = mass / 2;
        double new_radius = Constants::instance().RADIUS_FACTOR * qSqrt(new_mass);

        Player *new_player = new Player(id, x, y, new_radius, new_mass, max_fId + 1);
        new_player->set_impulse(SPLIT_START_SPEED, angle);
        copyPropertiesTo(new_player);

        fragmentId = max_fId + 2;
        fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        mass = new_mass;
        radius = new_radius;

        updateItems();

        return new_player;
    }

    bool can_fuse(const Player *frag) const {
        double dist = frag->calc_dist(x, y);
        double nR = radius + frag->getR();

        return fuse_timer == 0 && frag->fuse_timer == 0 && dist <= nR;
    }

    void collisionCalc(Player *other) {
        if (is_fast || other->is_fast) { // do not collide splits
            return;
        }
        double dist = this->calc_dist(other->x, other->y);
        if (dist >= radius + other->radius) {
            return;
        }

        // vector from centers
        double collisionVectorX = this->x - other->x;
        double collisionVectorY = this->y - other->y;
        // normalize to 1
        double vectorLen = qSqrt(collisionVectorX * collisionVectorX + collisionVectorY * collisionVectorY);
        if (vectorLen < 1e-9) { // collision object in same point??
            return;
        }
        collisionVectorX /= vectorLen;
        collisionVectorY /= vectorLen;

        double collisionForce = 1. - dist / (radius + other->radius);
        collisionForce *= collisionForce;
        collisionForce *= COLLISION_POWER;

        double sumMass = getM() + other->getM();
        // calc influence on us
        {
            double currPart = other->getM() / sumMass; // more influence on us if other bigger and vice versa

            double dx = speed * qCos(angle);
            double dy = speed * qSin(angle);
            dx += collisionForce * currPart * collisionVectorX;
            dy += collisionForce * currPart * collisionVectorY;
            this->speed = qSqrt(dx * dx + dy * dy);
            this->angle = qAtan2(dy, dx);
        }

        // calc influence on other
        {
            double otherPart = getM() / sumMass;

            double dx = other->speed * qCos(other->angle);
            double dy = other->speed * qSin(other->angle);
            dx -= collisionForce * otherPart * collisionVectorX;
            dy -= collisionForce * otherPart * collisionVectorY;
            other->speed = qSqrt(dx * dx + dy * dy);
            other->angle = qAtan2(dy, dx);
        }

        updateItems();
        other->updateItems();
    }

    void fusion(Player *frag) {
        double fragDX = frag->speed * qCos(frag->angle);
        double fragDY = frag->speed * qSin(frag->angle);
        double dX = speed * qCos(angle);
        double dY = speed * qSin(angle);
        double sumMass = mass + frag->mass;

        double fragInfluence = frag->mass / sumMass;
        double currInfluence = mass / sumMass;

        // center with both parts influence
        this->x = this->x * currInfluence + frag->x * fragInfluence;
        this->y = this->y * currInfluence + frag->y * fragInfluence;

        // new move vector with both parts influence
        dX = dX * currInfluence + fragDX * fragInfluence;
        dY = dY * currInfluence + fragDY * fragInfluence;

        // new angle and speed, based on vectors
        angle = qAtan2(dY, dX);
        speed = qSqrt(dX * dX + dY * dY);

        mass += frag->getM();
        updateItems();
    }

    bool can_eject() const {
        return mass > MIN_EJECT_MASS;
    }

    Ejection *eject_now(int eject_id) {
        double ex = x + qCos(angle) * (radius + 1);
        double ey = y + qSin(angle) * (radius + 1);

        Ejection *new_eject = new Ejection(eject_id, ex, ey, EJECT_RADIUS, EJECT_MASS, this->id);
        new_eject->set_impulse(EJECT_START_SPEED, angle);
        new_eject->setScene(m_scene);

        mass -= EJECT_MASS;
        radius = Constants::instance().RADIUS_FACTOR * qSqrt(mass);
        score += SCORE_FOR_EJECT;
        updateItems();
        return new_eject;
    }

    bool update_by_mass(int max_x, int max_y) {
        bool changed = false;
        double new_radius = Constants::instance().RADIUS_FACTOR * qSqrt(mass);
        if (radius != new_radius) {
            radius = new_radius;
            changed = true;
        }

        double new_speed = Constants::instance().SPEED_FACTOR / qSqrt(mass);
        if (speed > new_speed && !is_fast) {
            speed = new_speed;
        }

        if (x - radius < 0) {
            x += (radius - x);
            changed = true;
        }
        if (y - radius < 0) {
            y += (radius - y);
            changed = true;
        }
        if (x + radius > max_x) {
            x -= (radius + x - max_x);
            changed = true;
        }
        if (y + radius > max_y) {
            y -= (radius + y - max_y);
            changed = true;
        }

        if (changed) {
            updateItems();
        }

        return changed;
    }

    void apply_direct(Direct direct, int max_x, int max_y) {
        direct.limit();
        cmd_x = direct.x; cmd_y = direct.y;
        if (is_fast) return;

        double speed_x = speed * qCos(angle);
        double speed_y = speed * qSin(angle);
        double max_speed = Constants::instance().SPEED_FACTOR / qSqrt(mass);

        double dy = direct.y - y, dx = direct.x - x;
        double dist = qSqrt(dx * dx + dy * dy);
        double ny = (dist > 0)? (dy / dist) : 0;
        double nx = (dist > 0)? (dx / dist) : 0;
        double inertion = Constants::instance().INERTION_FACTOR;

        speed_x += (nx * max_speed - speed_x) * inertion / mass;
        speed_y += (ny * max_speed - speed_y) * inertion / mass;

        angle = qAtan2(speed_y, speed_x);

        double new_speed = qSqrt(speed_x*speed_x + speed_y*speed_y);
        if (new_speed > max_speed) {
            new_speed = max_speed;
        }
        speed = new_speed;
        updateItems();
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
        else {
            // зануляем проекцию скорости по dx
            double speed_y = speed * qSin(angle);
            speed = qAbs(speed_y);
            angle = (speed_y >= 0)? M_PI / 2 : -M_PI / 2;
        }
        if (dB + dy < max_y && uB + dy > 0) {
            y += dy;
            changed = true;
        }
        else {
            // зануляем проекцию скорости по dy
            double speed_x = speed * qCos(angle);
            speed = qAbs(speed_x);
            angle = (speed_x >= 0)? 0 : M_PI;
        }

        if (is_fast) {
            double max_speed = Constants::instance().SPEED_FACTOR / qSqrt(mass);
            apply_viscosity(max_speed);
        }
        if (fuse_timer > 0) {
            fuse_timer--;
        }
        if (changed) {
            updateItems();
        }
        return changed;
    }

    bool clear_fragments() {
        if (fragmentId == 0) {
            return false;
        }
        fragmentId = 0;
        return true;
    }

    bool can_shrink() const {
        return mass > MIN_SHRINK_MASS;
    }

    void shrink_now() {
        mass -= ((mass - MIN_SHRINK_MASS) * SHRINK_FACTOR);
        radius = Constants::instance().RADIUS_FACTOR * qSqrt(mass);
        updateItems();
    }

public:
    virtual QJsonObject toJson(bool mine=false) const {
        QJsonObject objData;
        objData.insert("Id", QJsonValue(id_to_str()));
        objData.insert("X", QJsonValue(x));
        objData.insert("Y", QJsonValue(y));
        objData.insert("M", QJsonValue(mass));
        objData.insert("R", QJsonValue(radius));
        if (mine) {
            objData.insert("SX", QJsonValue(speed * qCos(angle)));
            objData.insert("SY", QJsonValue(speed * qSin(angle)));
            if (fuse_timer > 0) {
                objData.insert("TTF", QJsonValue(fuse_timer));
            }
        }
        else {
            objData.insert("T", QJsonValue("P"));
        }
        return objData;
    }

private:
    QScopedPointer<QGraphicsEllipseItem> m_item;
    QScopedPointer<QGraphicsTextItem> m_textItem;
    QScopedPointer<QGraphicsLineItem> m_velocityLine;
    QScopedPointer<QGraphicsLineItem> m_commandLine;
    QScopedPointer<QGraphicsEllipseItem> m_visionLine;
};

typedef QVector<Player*> PlayerArray;

#endif // PLAYER_H
