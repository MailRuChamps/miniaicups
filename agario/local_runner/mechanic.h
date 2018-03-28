#ifndef MECHANIC_H
#define MECHANIC_H

#include <functional>
#include <QObject>
#include <QMap>

#include "logger.h"
#include "entities/food.h"
#include "entities/virus.h"
#include "entities/player.h"
#include "entities/ejection.h"

#include "strategies/strategy.h"
#include "strategies/bymouse.h"

typedef std::function<void(double, double)> AddFunc;
typedef std::function<Strategy*(Player*)> StrategyGet;


class Mechanic : public QObject
{
    Q_OBJECT

private:
    int tick;
    int id_counter;
    Logger *logger;

    FoodArray food_array;
    EjectionArray eject_array;
    VirusArray virus_array;

    PlayerArray player_array;
    StrategyArray strategy_array;
    QMap<int, int> player_scores;

    std::mt19937_64 rand;

public:
    explicit Mechanic() :
        tick(0),
        id_counter(1),
        logger(new Logger)
    {}

    virtual ~Mechanic() {
        clear_objects(false);
        if (logger) delete logger;
    }

public:
    void init_objects(quint64 seed, const StrategyGet &get_strategy) {
        rand.seed(seed);
        logger->init_file(QString::number(seed));

        add_player(START_PLAYER_SETS, get_strategy);
        add_food(START_FOOD_SETS);
        add_virus(START_VIRUS_SETS);

        write_base_tick();
    }

    void clear_objects(bool with_log=true) {
        tick = 0;
        if (with_log) {
            logger->clear_file();
        }

        id_counter = 0;
        for (Food *food : food_array) {
            if (food) delete food;
        }
        food_array.clear();

        for (Ejection *eject : eject_array) {
            if (eject) delete eject;
        }
        eject_array.clear();

        for (Virus *virus : virus_array) {
            if (virus) delete virus;
        }
        virus_array.clear();

        for (Player *player : player_array) {
            if (player) delete player;
        }
        player_array.clear();
        for (Strategy *strategy : strategy_array) {
            if (strategy) delete strategy;
        }
        strategy_array.clear();
    }

    void paintEvent(QPainter &painter, bool show_speed, bool show_fogs, bool show_cmd) {
        if (show_fogs) {
            for (Player *player : player_array) {
                player->draw_vision_line(painter);
            }
        }

        for (Food *food : food_array) {
            food->draw(painter);
        }
        for (Ejection *eject : eject_array) {
            eject->draw(painter);
        }
        std::sort(player_array.begin(), player_array.end(), [] (Player *lhs, Player *rhs) {
            return lhs->getR() < rhs->getR();
        });
        for (Player *player : player_array) {
            player->draw(painter, show_speed, show_cmd);
        }
        for (Virus *virus : virus_array) {
            virus->draw(painter);
        }
    }

    int tickEvent() {

#ifdef LOCAL_RUNNER
        apply_strategies();
#endif

        tick++;
        if (tick % SHRINK_EVERY_TICK == 0) {
            shrink_players();
        }
        who_is_eaten();
        who_need_fusion();
        who_intersected_virus();

        update_by_state();
        move_moveables();

        if (tick % ADD_FOOD_DELAY == 0 && food_array.length() < MAX_GAME_FOOD) {
            add_food(ADD_FOOD_SETS);
        }
        if (tick % ADD_VIRUS_DELAY == 0 && virus_array.length() < MAX_GAME_VIRUS) {
            add_virus(ADD_VIRUS_SETS);
        }
        if (tick % Constants::instance().BASE_TICK == 0) {
            write_base_tick();
        }
        return tick;
    }

    bool known() const {
        QVector<int> livingIds;
        for (Player *player : player_array) {
            int pId = player->getId();
            if (! livingIds.contains(pId)) {
                livingIds.append(pId);
            }
        }
        if (livingIds.length() == 0) {
            return true;
        }
        else if (livingIds.length() == 1) {
            int living_score = player_scores[livingIds[0]];
            for (int pId : player_scores.keys()) {
                if (pId != livingIds[0] && player_scores[pId] >= living_score) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    void mouseMoveEvent(int x, int y) {
        for (Strategy *strategy : strategy_array) {
            ByMouse *by_mouse = dynamic_cast<ByMouse*>(strategy);
            if (by_mouse != NULL) {
                by_mouse->set_mouse(x, y);
            }
        }
    }

    void keyPressEvent(QKeyEvent *event) {
        for (Strategy *strategy : strategy_array) {
            ByMouse *by_mouse = dynamic_cast<ByMouse*>(strategy);
            if (by_mouse != NULL) {
                by_mouse->set_key(event);
            }
        }
    }

public:
    void write_base_tick() {
        for (Food *food : food_array) {
            logger->write_add_cmd(tick, food);
        }
        for (Ejection *eject : eject_array) {
            logger->write_add_cmd(tick, eject);
        }
        for (Virus *virus : virus_array) {
            logger->write_add_cmd(tick, virus);
        }
        for (Player *player : player_array) {
            logger->write_add_cmd(tick, player);
        }
        for (int pId : player_scores.keys()) {
            logger->write_player_score(tick, pId, player_scores[pId]);
        }
    }

    Logger *get_logger() const {
        return logger;
    }

    bool is_space_empty(double _x, double _y, double _radius) const {
        for (Player *player : player_array) {
            if (player->is_intersected(_x, _y, _radius)) {
                return false;
            }
        }
        for (Virus *virus : virus_array) {
            if (virus->is_intersected(_x, _y, _radius)) {
                return false;
            }
        }
        return true;
    }

public:
    void add_circular(int sets_cnt, double one_radius, const AddFunc &add_one) {
        double center_x = Constants::instance().GAME_WIDTH / 2, center_y = Constants::instance().GAME_HEIGHT / 2;
        for (int I = 0; I < sets_cnt; I++) {
            double _x = rand() % qCeil(center_x - 4 * one_radius) + 2 * one_radius;
            double _y = rand() % qCeil(center_y - 4 * one_radius) + 2 * one_radius;

            add_one(_x, _y);
            add_one(center_x + (center_x - _x), _y);
            add_one(center_x + (center_x - _x), center_y + (center_y - _y));
            add_one(_x, center_y + (center_y - _y));
        }
    }

    void add_food(int sets_cnt) {
        add_circular(sets_cnt, FOOD_RADIUS, [=] (double _x, double _y) {
            Food *new_food = new Food(id_counter, _x, _y, FOOD_RADIUS, Constants::instance().FOOD_MASS);
            food_array.append(new_food);
            id_counter++;
            if (tick % Constants::instance().BASE_TICK != 0) {
                logger->write_add_cmd(tick, new_food);
            }
        });
    }

    void add_virus(int sets_cnt) {
        double rad = Constants::instance().VIRUS_RADIUS;
        add_circular(sets_cnt, rad, [=] (double _x, double _y) {
            if (! is_space_empty(_x, _y, rad)) {
                return;
            }
            Virus *new_virus = new Virus(id_counter, _x, _y, rad, VIRUS_MASS);
            virus_array.append(new_virus);
            id_counter++;
            if (tick % Constants::instance().BASE_TICK != 0) {
                logger->write_add_cmd(tick, new_virus);
            }
        });
    }

    void add_player(int sets_cnt, const StrategyGet &get_strategy) {
        bool by_mouse = true;
        add_circular(sets_cnt, PLAYER_RADIUS, [=, &by_mouse] (double _x, double _y) {
            if (! is_space_empty(_x, _y, PLAYER_RADIUS)) {
                return;
            }
            Player *new_player = new Player(id_counter, _x, _y, PLAYER_RADIUS, PLAYER_MASS);
            player_array.append(new_player);
            new_player->update_by_mass(Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT);

#ifdef LOCAL_RUNNER
            Strategy *new_strategy = get_strategy(new_player);
            strategy_array.append(new_strategy);
#endif

            player_scores[id_counter] = 0;
            id_counter++;
            if (tick % Constants::instance().BASE_TICK != 0) {
                logger->write_add_cmd(tick, new_player);
            }
        });
    }

    void delete_food(Food *food) {
        int rm_index = food_array.indexOf(food);
        if (rm_index != -1) {
            food_array.remove(rm_index);
            if (food) delete food;
        }
    }

    void delete_virus(Virus *virus) {
        int rm_index = virus_array.indexOf(virus);
        if (rm_index != -1) {
            virus_array.remove(rm_index);
            if (virus) delete virus;
        }
    }

    void delete_ejection(Ejection *eject) {
        int rm_index = eject_array.indexOf(eject);
        if (rm_index != -1) {
            eject_array.remove(rm_index);
            if (eject) delete eject;
        }
    }

    void delete_player(Player *player) {
        int pId = player->getId();
        PlayerArray other_players = get_players_by_id(pId);

        Strategy *strategy = get_strategy_by_id(pId);
        if (strategy != NULL && other_players.length() == 1) {
            int rm_index = strategy_array.indexOf(strategy);
            if (rm_index != -1) {
                strategy_array.remove(rm_index);
                if (strategy) delete strategy;
            }
        }
        int rm_index = player_array.indexOf(player);
        if (rm_index != -1) {
            player_array.remove(rm_index);
            if (player) delete player;
        }
    }

public:
    PlayerArray get_players_by_id(int pId) const {
        PlayerArray result;
        for (Player *player : player_array) {
            if (player->getId() == pId) {
                result.append(player);
            }
        }
        return result;
    }

    int get_fragments_cnt(int pId) const {
        int cnt = 0;
        for (Player *player : player_array) {
            if (player->getId() == pId) {
                cnt++;
            }
        }
        return cnt;
    }

    int get_max_fragment_id(int pId) const {
        int max_fId = 0;
        for (Player *player : player_array) {
            if (player->getId() == pId && max_fId < player->get_fId()) {
                max_fId = player->get_fId();
            }
        }
        return max_fId;
    }

    Strategy *get_strategy_by_id(int sId) const {
        for (Strategy *strategy : strategy_array) {
            if (strategy->getId() == sId) {
                return strategy;
            }
        }
        return NULL;
    }

    CircleArray get_visibles(const PlayerArray& for_them) const {
        // fog of war
        for (Player *player : player_array) {
            int frag_cnt = get_fragments_cnt(player->getId());
            bool updated = player->update_vision(frag_cnt);
            if (updated) {
                logger->write_fog_for(tick, player);
            }
        }

        CircleArray visibles;
        for (Player *fragment : for_them) {
            for (Food *food : food_array) {
                if (fragment->can_see(food)) {
                    visibles.append((Circle *) food);
                }
            }
            for (Ejection *eject : eject_array) {
                if (fragment->can_see(eject)) {
                    visibles.append((Circle *) eject);
                }
            }
            for (Player *player : player_array) {
                if (for_them.indexOf(player) == -1) {
                    if (fragment->can_see(player)) {
                        visibles.append((Circle *) player);
                    }
                }
            }
        }
        for (Virus *virus : virus_array) {
            visibles.append((Circle *) virus);
        }
        return visibles;
    }

public:
    void apply_strategies() {
        for (Strategy *strategy : strategy_array) {
            int sId = strategy->getId();
            PlayerArray fragments = get_players_by_id(sId);
            CircleArray visibles = get_visibles(fragments);

            Direct direct = strategy->tickEvent(fragments, visibles);
//            logger->write_direct(tick, sId, direct);

            int yet_cnt = fragments.length();
            for (Player *frag : fragments) {
                frag->apply_direct(direct, Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT);
                logger->write_direct_for(tick, frag);

                if (direct.split && frag->can_split(yet_cnt)) {
                    frag->logical = Player::SPLIT;
                    yet_cnt++;
                }
                if (direct.eject && frag->can_eject()) {
                    frag->logical = Player::EJECT;
                }
            }
        }
    }

    void apply_direct_for(int sId, Direct direct) {
//        logger->write_direct(tick, sId, direct);

        PlayerArray fragments = get_players_by_id(sId);
        int yet_cnt = fragments.length();

        for (Player *frag : fragments) {
            frag->apply_direct(direct, Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT);
            logger->write_direct_for(tick, frag);

            if (direct.split && frag->can_split(yet_cnt)) {
                frag->logical = Player::SPLIT;
            }
            if (direct.eject && frag->can_eject()) {
                frag->logical = Player::EJECT;
            }
        }
    }

    void who_is_eaten() {
        auto nearest_player = [this] (Circle *circle) {
            Player *nearest_predator = NULL;
            double nearest_dist = INFINITY;
            for (Player *predator : player_array) {
                double qdist = predator->can_eat(circle);
                if (qdist < nearest_dist) {
                    nearest_dist = qdist;
                    nearest_predator = predator;
                }
            }
            return nearest_predator;
        };
        auto nearest_virus = [this] (Ejection *eject) {
            Virus *nearest_predator = NULL;
            double nearest_dist = INFINITY;
            for (Virus *predator : virus_array) {
                double qdist = predator->can_eat(eject);
                if (qdist < nearest_dist) {
                    nearest_dist = qdist;
                    nearest_predator = predator;
                }
            }
            return nearest_predator;
        };

        for (Food *food : food_array) {
            if (food->logical != Food::EATEN) {
                Player *eater = nearest_player(food);
                if (eater != NULL) {
                    eater->eat(food);
                    food->logical = Food::EATEN;
                }
            }
        }
        for (Ejection *eject : eject_array) {
            if (eject->logical != Ejection::EATEN) {
                Virus *eater = nearest_virus(eject);
                if (eater != NULL) {
                    eater->eat(eject);
                    if (eater->can_split()) {
                        eater->logical = Virus::SPLIT;
                    }
                    eject->logical = Ejection::EATEN;
                }
                else {
                    Player *eater = nearest_player(eject);
                    if (eater != NULL) {
                        eater->eat(eject);
                        eject->logical = Ejection::EATEN;
                    }
                }
            }
        }
        for (Player *player : player_array) {
            if (player->logical != Player::EATEN) {
                Player *eater = nearest_player(player);
                if (eater != NULL) {
                    bool is_last = get_fragments_cnt(player->getId()) == 1;
                    eater->eat(player, is_last);
                    player->logical = Player::EATEN;
                }
            }
        }
    }

    void who_intersected_virus() {
        auto nearest_to = [this] (Virus *virus) {
            double nearest_dist = INFINITY;
            Player *nearest_player = NULL;

            for (Player *player : player_array) {
                if (player->logical == Player::CREATED) {
                    double qdist = virus->can_hurt(player);
                    if (qdist < nearest_dist) {

                        int yet_cnt = get_fragments_cnt(player->getId());
                        if (player->can_burst(yet_cnt)) {
                            nearest_dist = qdist;
                            nearest_player = player;
                        }
                    }
                }
            }
            return nearest_player;
        };

        for (Virus *virus : virus_array) {
            if (virus->logical == Virus::CREATED || virus->logical == Virus::MOVING) {
                Player *nearest_player = nearest_to(virus);
                if (nearest_player) {
                    nearest_player->burst_on(virus);
                    virus->logical = Virus::HURT;
                }
            }
        }
    }

    void who_need_fusion() {
        for (Player *player : player_array) {
            PlayerArray fragments = get_players_by_id(player->getId());
            if (fragments.length() == 1) {

                QString old_id = player->id_to_str();
                bool changed = player->clear_fragments();
                if (changed) {
                    logger->write_change_id(tick, old_id, player);
                }
            }

            for (Player *frag : fragments) {
                if (player != frag && frag->logical != Player::FUSED) {
                    if (player->can_fuse(frag)) {
                        player->fusion(frag);
                        frag->logical = Player::FUSED;
                    }
                }
            }
        }
    }

    void move_moveables() {
        Constants &ins = Constants::instance();
        for (Ejection *eject : eject_array) {
            if (eject->logical == Ejection::MOVING) {
                bool changed = eject->move(ins.GAME_WIDTH, ins.GAME_HEIGHT);
                if (changed) {
                    logger->write_change_pos(tick, eject);
                }
            }
        }
        for (Virus *virus : virus_array) {
            if (virus->logical == Virus::MOVING) {
                bool changed = virus->move(ins.GAME_WIDTH, ins.GAME_HEIGHT);
                if (changed) {
                    logger->write_change_pos(tick, virus);
                }
            }
        }
        for (Player *player : player_array) {
            bool changed = player->move(ins.GAME_WIDTH, ins.GAME_HEIGHT);
            if (changed) {
                logger->write_change_pos(tick, player);
            }
        }
    }

    void update_by_state() {
        EjectionArray remove_ejects;
        for (Ejection *eject : eject_array) {
            if (eject->logical == Ejection::EATEN) {
                logger->write_kill_cmd(tick, eject);
                remove_ejects.append(eject);
            }
        }
        for (Ejection *eject : remove_ejects) {
            delete_ejection(eject);
        }

        PlayerArray remove_players;
        PlayerArray append_players;
        for (Player *player : player_array) {
            if (player->logical == Player::EATER || player->logical == Player::FUSER) {
                bool changed = player->update_by_mass(Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT);
                if (changed) {
                    logger->write_change_mass(tick, player);
                }
            }
            else if (player->logical == Player::EATEN || player->logical == Player::FUSED) {
                logger->write_kill_cmd(tick, player);
                remove_players.append(player);
            }
            else if (player->logical == Player::BURST || player->logical == Player::SPLIT) {
                int yet_cnt = get_fragments_cnt(player->getId());
                int max_fId = get_max_fragment_id(player->getId());
                QString old_id = player->id_to_str();

                PlayerArray fragments;
                if (player->logical == Player::BURST) {
                    fragments = player->burst_now(max_fId, yet_cnt);
                }
                else if (player->logical == Player::SPLIT) {
                    Player *new_player = player->split_now(max_fId);
                    fragments.append(new_player);
                }
                if (fragments.length() > 0) {
                    for (Player *frag : fragments) {
                        logger->write_add_cmd(tick, frag);
                        append_players.append(frag);
                    }
                    logger->write_change_mass_id(tick, old_id, player);
                }
            }
            else if (player->logical == Player::EJECT) {
                Ejection *new_eject = player->eject_now(id_counter);
                logger->write_add_cmd(tick, new_eject);
                eject_array.append(new_eject);
                id_counter++;
            }

            int score = player->get_score();
            if (score > 0) {
                int pId = player->getId();
                player_scores[pId] += score;
                logger->write_player_score(tick, pId, player_scores[pId]);
            }
        }
        for (Player *player : append_players) {
            player_array.append(player);
        }
        for (Player *player : remove_players) {
            delete_player(player);
        }

        VirusArray remove_viruses;
        for (Virus *virus : virus_array) {
            if (virus->logical == Virus::HURT) {
                logger->write_kill_cmd(tick, virus);
                remove_viruses.append(virus);
            }
            else if (virus->logical == Virus::EATER) {
                virus->logical = Virus::CREATED;
            }
            else if (virus->logical == Virus::SPLIT) {
                Virus *new_virus = virus->split_now(id_counter);
                logger->write_add_cmd(tick, new_virus);
                virus_array.append(new_virus);
                id_counter++;
            }
        }
        for (Virus *virus : remove_viruses) {
            delete_virus(virus);
        }

        FoodArray remove_food;
        for (Food *food : food_array) {
            if (food->logical == Food::EATEN) {
                logger->write_kill_cmd(tick, food);
                remove_food.append(food);
            }
        }
        for (Food *food : remove_food) {
            delete_food(food);
        }
    }

    void shrink_players() {
        for (Player *player : player_array) {
            if (player->can_shrink()) {
                player->shrink_now();
                logger->write_change_mass(tick, player);
            }
        }
    }

    int get_score_for(int pId) const {
        return player_scores[pId];
    }

    QMap<int, int> get_scores() const {
        return player_scores;
    }
};

#endif // MECHANIC_H
