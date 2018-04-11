#ifndef PLAYER_H
#define PLAYER_H

#include "circle.h"
#include "ejection.h"


class Player : public Circle
{
public:
    bool is_fast;
    int fuse_timer;

protected:
    double speed, angle;
    int fragmentId;
    int color;
    double vision_radius;
    double cmd_x, cmd_y;

public:
    explicit Player(int _id, double _x, double _y, double _radius, double _mass, const int fId=0) :
        Circle(_id, _x, _y, _radius, _mass),
        is_fast(false),
        fuse_timer(0),
        speed(0), angle(0),
        fragmentId(fId),
        vision_radius(0),
        cmd_x(0), cmd_y(0)
    {
        color = _id + 7;
        if (fId > 0) {
            fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        }
    }

    virtual ~Player() {}

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
    }

    virtual bool is_player() const {
        return true;
    }

    QPair<double, double> get_direct() const {
        return QPair<double, double>(cmd_x, cmd_y);
    }

    double getVR() const {
        return vision_radius;
    }

public:
    void set_impulse(double new_speed, double new_angle) {
        speed = qAbs(new_speed);
        angle = new_angle;
        is_fast = true;
    }

    void apply_viscosity(double usual_speed) {
        // если на этом тике не снизим скорость достаточно - летим дальше
        if (speed - Constants::instance().VISCOSITY > usual_speed) {
            speed -= Constants::instance().VISCOSITY;
        } else {
            // иначе выставляем максимальную скорость и выходим из режима полёта
            speed = usual_speed;
            is_fast = false;
        }
    }

    void draw(QPainter &painter, bool show_speed=false, bool show_cmd=false) const {
        painter.setPen(QPen(QBrush(Qt::black), 1));
        painter.setBrush(Qt::GlobalColor(color));

        int ix = int(x), iy = int(y), ir = int(radius);
        painter.drawEllipse(QPoint(ix, iy), ir, ir);
        painter.drawText(ix - 4, iy + 4, QString::number(mass));

        if (show_speed) {
            painter.setPen(QPen(QBrush(Qt::green), 1));
            int speed_x = ix + speed * qCos(angle) * DRAW_SPEED_FACTOR;
            int speed_y = iy + speed * qSin(angle) * DRAW_SPEED_FACTOR;
            painter.drawLine(ix, iy, speed_x, speed_y);
        }
        if (show_cmd) {
            painter.setPen(QPen(QBrush(Qt::red), 1));
            QPair<double, double> norm = get_direct();
            painter.drawLine(ix, iy, norm.first, norm.second);
        }
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
            return true;
        }
        return false;
    }

    bool can_see(const Circle *circle) {
        double xVisionCenter = x + qCos(angle) * VIS_SHIFT;
        double yVisionCenter = y + qSin(angle) * VIS_SHIFT;
        double qdist = circle->calc_qdist(xVisionCenter, yVisionCenter);

        return (qdist < (vision_radius + circle->getR()) * (vision_radius + circle->getR()));
    }

    void draw_vision_ellipse(QPainter &painter) const {
        double xVisionCenter = x + qCos(angle) * VIS_SHIFT;
        double yVisionCenter = y + qSin(angle) * VIS_SHIFT;

        painter.drawEllipse(QPointF(xVisionCenter, yVisionCenter), vision_radius, vision_radius);
    }

    void clear_vision_area(QPainter &painter) const {
        painter.setPen(Qt::white);
        painter.setBrush(Qt::white);

        draw_vision_ellipse(painter);
    }

    void draw_vision_line(QPainter &painter) const {
        painter.setPen(QPen(QBrush(Qt::black), 1, Qt::DashLine));
        painter.setBrush(Qt::transparent);

        draw_vision_ellipse(painter);
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

    void eat(Circle *food) {
        mass += food->getM();
    }

    bool can_burst(int yet_cnt) {
        if (mass < MIN_BURST_MASS * 2) {
            return false;
        }
        int frags_cnt = int(mass / MIN_BURST_MASS);
        if (frags_cnt > 1 && rest_fragments_count(yet_cnt) > 0) {
            return true;
        }
        return false;
    }

    void burst_on(Circle *virus) {
        double dy = y - virus->getY(), dx = x - virus->getX();

        angle = qAtan2(dy, dx);
        double max_speed = Constants::instance().SPEED_FACTOR / qSqrt(mass);
        if (speed < max_speed) {
            speed = max_speed;
        }
        mass += BURST_BONUS;
    }

    QVector<Player*> burst_now(int max_fId, int yet_cnt) {
        QVector<Player*> fragments;
        int new_frags_cnt = int(mass / MIN_BURST_MASS) - 1;

        new_frags_cnt = std::min(new_frags_cnt, rest_fragments_count(yet_cnt));

        double new_mass = mass / (new_frags_cnt + 1);
        double new_radius = mass2radius(new_mass);

        for (int I = 0; I < new_frags_cnt; I++) {
            int new_fId = max_fId + I + 1;
            Player *new_fragment = new Player(id, x, y, new_radius, new_mass, new_fId);
            new_fragment->set_color(color);
            fragments.append(new_fragment);

            double burst_angle = angle - BURST_ANGLE_SPECTRUM / 2 + I * BURST_ANGLE_SPECTRUM / new_frags_cnt;
            new_fragment->set_impulse(BURST_START_SPEED, burst_angle);
        }
        set_impulse(BURST_START_SPEED, angle + BURST_ANGLE_SPECTRUM / 2);

        fragmentId = max_fId + new_frags_cnt + 1;
        mass = new_mass;
        radius = new_radius;
        fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        return fragments;
    }

    bool can_split(int yet_cnt) {

        if (rest_fragments_count(yet_cnt) > 0) {

            if (mass > MIN_SPLIT_MASS) {
                return true;
            }
        }
        return false;
    }

    Player *split_now(int max_fId) {
        double new_mass = mass / 2;
        double new_radius = mass2radius(new_mass);

        Player *new_player = new Player(id, x, y, new_radius, new_mass, max_fId + 1);
        new_player->set_color(color);
        new_player->set_impulse(SPLIT_START_SPEED, angle);

        fragmentId = max_fId + 2;
        fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        mass = new_mass;
        radius = new_radius;

        return new_player;
    }

    bool can_fuse(Player *frag) {
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
    }

    bool can_eject() {
        return mass > MIN_EJECT_MASS;
    }

    Ejection *eject_now(int eject_id) {
        double ex = x + qCos(angle) * (radius + 1);
        double ey = y + qSin(angle) * (radius + 1);

        Ejection *new_eject = new Ejection(eject_id, ex, ey, EJECT_RADIUS, EJECT_MASS, this->id);
        new_eject->set_impulse(EJECT_START_SPEED, angle);

        mass -= EJECT_MASS;
        radius = mass2radius(mass);
        return new_eject;
    }

    bool update_by_mass(int max_x, int max_y) {
        bool changed = false;
        double new_radius = mass2radius(mass);
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
            // долетаем до стенки
            double new_x = qMax(radius, qMin(max_x - radius, x + dx));
            changed |= (x != new_x);
            x = new_x;
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
            // долетаем до стенки
            double new_y = qMax(radius, qMin(max_y - radius, y + dy));
            changed |= (y != new_y);
            y = new_y;
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
        return changed;
    }

    bool clear_fragments() {
        if (fragmentId == 0) {
            return false;
        }
        fragmentId = 0;
        return true;
    }

    bool can_shrink() {
        return mass > MIN_SHRINK_MASS;
    }

    void shrink_now() {
        mass -= ((mass - MIN_SHRINK_MASS) * SHRINK_FACTOR);
        radius = mass2radius(mass);
    }
    double get_speed() {
        return speed;
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

    static double mass2radius(double mass) {
        return PLAYER_RADIUS_FACTOR * std::sqrt(mass);
    }
private:
    /**
     * @param existingFragmentsCount число фрагментов игрока
     * @return максимально возможное число фрагментов, которое может дополнительно появиться у игрока в результате
     * взрыва / деления
     */
    static int rest_fragments_count(const int existingFragmentsCount) {
        return Constants::instance().MAX_FRAGS_CNT - existingFragmentsCount;
    }
};

typedef QVector<Player*> PlayerArray;

#endif // PLAYER_H
