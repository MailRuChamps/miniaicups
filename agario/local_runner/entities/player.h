#ifndef PLAYER_H
#define PLAYER_H

#include "circle.h"
#include "ejection.h"


class Player : public Circle
{
public:
    enum State { CREATED, EATEN, EATER, BURST, FUSED, FUSER, SPLIT, EJECT };
    State logical;
    bool is_fast;
    int fuse_timer;

protected:
    double speed, angle;
    int fragmentId;
    int color;
    int score;
    double vision_radius;
    int cmd_x, cmd_y;

public:
    explicit Player(int _id, double _x, double _y, double _radius, double _mass, const int fId=0) :
        Circle(_id, _x, _y, _radius, _mass),
        logical(State::CREATED),
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

    int get_score() {
        int _score = score;
        score = 0;
        return _score;
    }

    QPair<double, double> get_direct_norm() const {
        double dx = cmd_x - x, dy = cmd_y - y;
        double dist = qSqrt(dx * dx + dy * dy);
        if (dist > 0) {
            double factor = 50 / dist;
            return QPair<double, double>(x + dx * factor, y + dy * factor);
        }
        return QPair<double, double>(x, y);
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
        if (is_fast && speed > usual_speed) {
            speed -= Constants::instance().VISCOSITY;
        }
        if (is_fast && speed < usual_speed) {
            is_fast = false;
            speed = usual_speed;
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
            QPair<double, double> norm = get_direct_norm();
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
        double qdist = circle->calc_qdist(x, y);
        if (qdist < vision_radius * vision_radius) {
            return true;
        }
        return false;
    }

    void draw_vision(QPainter &painter) const {
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::lightGray);
        int ix = int(x), iy = int(y);
        int ivr = int(vision_radius);

        int xShift = int(qCos(angle) * VIS_SHIFT), yShift = int(qSin(angle) * VIS_SHIFT);
        int arcX = ix + xShift, arcY = iy + yShift;
        painter.drawEllipse(QPoint(arcX, arcY), ivr, ivr);
    }

    void draw_vision_line(QPainter &painter) const {
        painter.setPen(QPen(QBrush(Qt::black), 1, Qt::DashLine));
        int ix = int(x), iy = int(y), ir = int(radius);
        int ivr = int(vision_radius);

        int xShift = int(qCos(angle) * VIS_SHIFT), yShift = int(qSin(angle) * VIS_SHIFT);
        int arcX = ix - ivr + xShift, arcY = iy - ivr + yShift;
        painter.drawArc(arcX, arcY, ivr * 2, ivr * 2, 0, 360 * 16);
    }

    double can_eat(Circle *food) const {
        if (food->is_player() && food->getId() == id) {
            return INFINITY;
        }
        if (mass > food->getM() * MASS_EAT_FACTOR) {
            double qdist = food->calc_qdist(x, y);
            double tR = radius - food->getR() * RAD_EAT_FACTOR;
            if (qdist < tR * tR) {
                return qdist;
            }
        }
        return INFINITY;
    }

    void eat(Circle *food, bool is_last=false) {
        mass += food->getM();
        logical = State::EATER;

        if (food->is_food()) {
            score += SCORE_FOR_FOOD;
        }
        else if (food->is_player()) {
            score += (! is_last)? SCORE_FOR_PLAYER : SCORE_FOR_LAST;
        }
    }

    bool can_burst(int yet_cnt) {
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
        logical = State::BURST;
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
            new_fragment->set_color(color);
            fragments.append(new_fragment);

            double burst_angle = angle - BURST_ANGLE_SPECTRUM / 2 + I * BURST_ANGLE_SPECTRUM / new_frags_cnt;
            new_fragment->set_impulse(BURST_START_SPEED, burst_angle);
        }
        set_impulse(BURST_START_SPEED, angle + BURST_ANGLE_SPECTRUM / 2);
        logical = State::CREATED;

        fragmentId = max_fId + new_frags_cnt + 1;
        mass = new_mass;
        radius = new_radius;
        fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        return fragments;
    }

    bool can_split(int yet_cnt) {
        if (logical != State::CREATED) {
            return false;
        }
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
        new_player->set_color(color);
        new_player->set_impulse(SPLIT_START_SPEED, angle);

        fragmentId = max_fId + 2;
        fuse_timer = Constants::instance().TICKS_TIL_FUSION;
        mass = new_mass;
        radius = new_radius;

        logical = State::CREATED;
        return new_player;
    }

    double tangent_projection(Player *c1, Player *c2, double dist) {
        if (dist > 0 && c1->speed > 0) {
            double speed_x = c1->speed * qCos(c1->angle);
            double speed_y = c1->speed * qSin(c1->angle);

            double norm_x = -(c1->y - c2->y), norm_y = c1->x - c2->x;
            double scalar = norm_x * speed_x + norm_y * speed_y;
            double cos_beta = scalar / dist / c1->speed;
            return qAcos(cos_beta);
        }
        return 0.0;
    }

    bool can_fuse(Player *frag) {
        double dist = frag->calc_dist(x, y);
        double nR = radius + frag->getR();

        if (frag->fuse_timer > 0 && dist <= nR && !is_fast) {
            double beta = tangent_projection(this, frag, dist);
            if (qAbs(beta) < M_PI / 2) {
                angle -= beta;
            }
//            speed *= qCos(beta);
        }
        if (frag->getM() < mass) {
            if (frag->fuse_timer == 0 && dist <= nR) {
                return true;
            }
        }
        return false;
    }

    void fusion(Player *frag) {
        mass += frag->getM();
        logical = State::FUSER;
    }

    bool can_eject() {
        if (logical != State::CREATED) {
            return false;
        }
        if (mass > MIN_EJECT_MASS) {
            return true;
        }
        return false;
    }

    Ejection *eject_now(int eject_id) {
        double ex = x + qCos(angle) * (radius + 1);
        double ey = y + qSin(angle) * (radius + 1);

        Ejection *new_eject = new Ejection(eject_id, ex, ey, EJECT_RADIUS, EJECT_MASS);
        new_eject->set_impulse(EJECT_START_SPEED, angle);

        mass -= EJECT_MASS;
        radius = Constants::instance().RADIUS_FACTOR * qSqrt(mass);
        logical = State::CREATED;
        score += SCORE_FOR_EJECT;
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

        logical = State::CREATED;
        return changed;
    }

    void apply_direct(Direct direct, int max_x, int max_y) {
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

        if (speed_y != 0 && speed_x != 0) {
            if (speed_x > 0) {
                angle = qAtan(speed_y / qAbs(speed_x));
            } else {
                angle = M_PI - qAtan(speed_y / qAbs(speed_x));
            }
        } else {
            angle = (speed_x >= 0)? 0 : M_PI;
        }

        double new_speed = qSqrt(speed_x*speed_x + speed_y*speed_y);
        if (new_speed > max_speed) {
            new_speed = max_speed;
        }
        speed = new_speed;
        if (dist < STOP_LIMIT) {
            speed = 0;

            double rB = x + radius, lB = x - radius;
            double dB = y + radius, uB = y - radius;
            if (rB < max_x && lB > 0) {
                x = cmd_x;
            }
            if (dB < max_y && uB > 0) {
                y = cmd_y;
            }
        }
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
        if (mass > MIN_SHRINK_MASS) {
            if (logical == State::CREATED) {
                return true;
            }
        }
        return false;
    }

    void shrink_now() {
        mass -= ((mass - MIN_SHRINK_MASS) * SHRINK_FACTOR);
        radius = Constants::instance().RADIUS_FACTOR * qSqrt(mass);
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
};

typedef QVector<Player*> PlayerArray;

#endif // PLAYER_H
