#ifndef MECHANIC_H
#define MECHANIC_H

#include <functional>
#include <QObject>
#include <QMap>
#include <list>
#include <array>

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
    QMap<int, Direct> strategy_directs;
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
    void init_objects(const std::string &seed, const StrategyGet &get_strategy) {
        std::seed_seq seq(seed.begin(), seed.end());
        rand.seed(seq);

        std::array<uint, 1> simple_seeds;
        seq.generate(simple_seeds.begin(), simple_seeds.end());
        srand(simple_seeds[0]); // на всякий случай, если вдруг где-то когда-то будет использоваться обычный rand().
                                // он используется, например, в умолчальной стратегии
        logger->init_file(QString::number(simple_seeds[0]), LOG_FILE, false);

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

        id_counter = 1;
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

    bool isSeenBySomeone(Circle *target, const QMap<int, bool> &player_vision){
        for (Player *player : player_array) {
            if (player_vision.value(player->getId()) && player->can_see(target)){
                return true;
            }
        }
        return false;
    }

    void paintEvent(QPainter &painter, bool show_speed, bool show_fogs, bool show_cmd, const QMap<int, bool> &player_vision) {
        bool fullVision = !player_vision.values().contains(true);

        if (!fullVision) {
            //draw fog everywhere
            painter.save();
            painter.setBrush(Qt::GlobalColor(Qt::gray));
            painter.fillRect(0, 0, Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT, Qt::Dense6Pattern);
            painter.restore();

            //clear fog for players with vision enabled
            for (Player *player : player_array) {
                if (player_vision.value(player->getId()))
                    player->clear_vision_area(painter);
            }

        }

        if (show_fogs) {
            for (Player *player : player_array) {
                if (fullVision || isSeenBySomeone(player, player_vision))
                    player->draw_vision_line(painter);
            }
        }

        for (Food *food : food_array) {
            if (fullVision || isSeenBySomeone(food, player_vision))
                food->draw(painter);
        }
        for (Ejection *eject : eject_array) {
            if (fullVision || isSeenBySomeone(eject, player_vision))
                eject->draw(painter);
        }
        std::sort(player_array.begin(), player_array.end(), [] (Player *lhs, Player *rhs) {
            return lhs->getR() < rhs->getR();
        });
        for (Player *player : player_array) {
            if (fullVision || player_vision.value(player->getId()) || isSeenBySomeone(player, player_vision))
                player->draw(painter, show_speed, show_cmd);
        }
        for (Virus *virus : virus_array) {
            virus->draw(painter);
        }
    }

    int tickEvent() {
        auto oldScores = player_scores;
#ifdef LOCAL_RUNNER
        apply_strategies();
#endif
        tick++;
        move_moveables();
        player_ejects();
        player_splits();

        if (tick % SHRINK_EVERY_TICK == 0) {
            shrink_players();
        }
        eat_all();
        fuse_players();
        burst_on_viruses();

        update_players_radius();

        for(auto sit = player_scores.begin(); sit != player_scores.end(); sit++) {
            if (oldScores[sit.key()] != sit.value()) {
                logger->write_player_score(tick, sit.key(), sit.value());
            }
        }

        split_viruses();

        if (tick % ADD_FOOD_DELAY == 0 && food_array.length() < MAX_GAME_FOOD) {
            add_food(ADD_FOOD_SETS);
        }
        if (tick % ADD_VIRUS_DELAY == 0 && virus_array.length() < MAX_GAME_VIRUS) {
            add_virus(ADD_VIRUS_SETS);
        }
        if (tick % Constants::instance().BASE_TICK == 0) {
            write_base_tick();
        }
        strategy_directs.clear();
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

        auto can_see = [&for_them](Circle* c){
            for (Player *fragment : for_them) {
                if (fragment->can_see(c)) {
                    return true;
                }
            }
            return false;
        };

        CircleArray visibles;
        for (Food *food : food_array) {
            if (can_see(food)) {
                visibles.append(food);
            }
        }
        for (Ejection *eject : eject_array) {
            if (can_see(eject)) {
                visibles.append(eject);
            }
        }
        auto pId = for_them.empty() ? -1 : for_them.front()->getId();
        for (Player *player : player_array) {
            if (player->getId() != pId && can_see(player)) {
                visibles.append(player);
            }
        }
        for (Virus *virus : virus_array) {
            visibles.append(virus);
        }
        return visibles;
    }

public:
    void apply_strategies() {
        for (Strategy *strategy : strategy_array) {
            int sId = strategy->getId();
            PlayerArray fragments = get_players_by_id(sId);
            if (fragments.empty()) {
                continue;
            }
            CircleArray visibles = get_visibles(fragments);

            Direct direct = strategy->tickEvent(fragments, visibles);
//            logger->write_direct(tick, sId, direct);

            apply_direct_for(sId, direct);
        }
    }

    void apply_direct_for(int sId, Direct direct) {
//        logger->write_direct(tick, sId, direct);
        PlayerArray fragments = get_players_by_id(sId);
        int yet_cnt = fragments.length();

        for (Player *frag : fragments) {
            frag->apply_direct(direct, Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT);

            logger->write_direct_for(tick, frag, direct);
        }

        strategy_directs.insert(sId, direct);
    }

    void split_fragments(PlayerArray fragments) {
        int fragments_count = fragments.length();

        // Сортировка фрагментов по массе. При совпадении массы - по индексу.
        // Фрагменты с большим значением критерия после сортировки окажутся ближе к началу.
        std::sort(fragments.begin(), fragments.end(), [] (const Player* lhs, const Player* rhs) {
            return
                std::make_tuple(lhs->getM(), lhs->get_fId()) >
                std::make_tuple(rhs->getM(), rhs->get_fId());
        });

        for (Player *frag : fragments) {

            if (frag->can_split(fragments_count)) {
                int max_fId = get_max_fragment_id(frag->getId());
                QString old_id = frag->id_to_str();

                Player *new_frag= frag->split_now(max_fId);
                player_array.push_back(new_frag);
                fragments_count++;

                logger->write_add_cmd(tick, new_frag);
                logger->write_change_mass_id(tick, old_id, frag);
            }
        }
    }

    void player_splits() {

        for (auto it = strategy_directs.begin(); it != strategy_directs.end(); it++) {
            const Direct& direct = it.value();

            if (direct.split) {
                const int player_id = it.key();
                const PlayerArray fragments = get_players_by_id(player_id);
                split_fragments(fragments);
            }
        }
    }

    void player_ejects() {
        for (auto it = strategy_directs.begin(); it != strategy_directs.end(); it++) {
            int sId = it.key();
            Direct direct = it.value();
            if(direct.split || !direct.eject) {
                continue;
            }
            PlayerArray fragments = get_players_by_id(sId);

            for (Player *frag : fragments) {
                if (frag->can_eject()) {
                    Ejection *new_eject = frag->eject_now(id_counter);
                    eject_array.append(new_eject);
                    id_counter++;

                    logger->write_add_cmd(tick, new_eject);
                }
            }
        }
    }

    void eat_all() {
        auto nearest_player = [this] (Circle *circle) {
            Player *nearest_predator = NULL;
            double deeper_dist = -INFINITY;
            for (Player *predator : player_array) {
                double qdist = predator->can_eat(circle);
                if (qdist > deeper_dist) {
                    deeper_dist = qdist;
                    nearest_predator = predator;
                }
            }
            return nearest_predator;
        };
        auto nearest_virus = [this] (Ejection *eject) {
            Virus *nearest_predator = NULL;
            double deeper_dist = -INFINITY;
            for (Virus *predator : virus_array) {
                double qdist = predator->can_eat(eject);
                if (qdist > deeper_dist) {
                    deeper_dist = qdist;
                    nearest_predator = predator;
                }
            }
            return nearest_predator;
        };

        for (auto fit = food_array.begin(); fit != food_array.end();) {
            if (Player *eater = nearest_player(*fit)) {
                eater->eat(*fit);
                player_scores[eater->getId()] += SCORE_FOR_FOOD;
                logger->write_kill_cmd(tick, *fit);
                delete *fit;
                fit = food_array.erase(fit);
            } else {
                fit++;
            }
        }

        for (auto eit = eject_array.begin(); eit != eject_array.end(); ) {
            auto eject = *eit;
            if (Virus *eater = nearest_virus(eject)) {
                eater->eat(eject);
            } else if (Player *eater = nearest_player(eject)) {
                eater->eat(eject);
                if (!eject->is_my_eject(eater)) {
                    player_scores[eater->getId()] += SCORE_FOR_FOOD;
                }
            } else {
                eit++;
                continue;
            }

            logger->write_kill_cmd(tick, eject);
            delete eject;
            eit = eject_array.erase(eit);
        }

        for (auto pit = player_array.begin(); pit != player_array.end(); ) {
            if(Player *eater = nearest_player(*pit)) {
                bool is_last = get_fragments_cnt((*pit)->getId()) == 1;
                eater->eat(*pit);
                player_scores[eater->getId()] += is_last? SCORE_FOR_LAST : SCORE_FOR_PLAYER;
                logger->write_kill_cmd(tick, *pit);
                delete *pit;
                pit = player_array.erase(pit);
            } else {
                pit++;
            }
        }
    }

    void burst_on_viruses() { // TODO: improve target selection
        PlayerArray targets = player_array;

        auto nearest_to = [this, &targets] (Virus *virus) {
            double nearest_dist = INFINITY;
            Player *nearest_player = NULL;

            for (Player *player : player_array) {
                double qdist = virus->can_hurt(player);
                if (qdist < nearest_dist) {
                    int yet_cnt = get_fragments_cnt(player->getId());
                    if (player->can_burst(yet_cnt)) {
                        nearest_dist = qdist;
                        nearest_player = player;
                    }
                }
            }
            return nearest_player;
        };



        for (auto vit = virus_array.begin(); vit != virus_array.end(); ) {
            if (Player *player = nearest_to(*vit)) {
                int yet_cnt = get_fragments_cnt(player->getId());
                int max_fId = get_max_fragment_id(player->getId());
                QString old_id = player->id_to_str();

                player->burst_on(*vit);
                player_scores[player->getId()] += SCORE_FOR_BURST;
                PlayerArray fragments = player->burst_now(max_fId, yet_cnt);
                player_array.append(fragments);
                targets.removeAll(player);

                for (Player *frag : fragments) {
                    logger->write_add_cmd(tick, frag);
                }
                logger->write_change_mass_id(tick, old_id, player);
                logger->write_kill_cmd(tick, *vit);
                delete *vit;
                vit = virus_array.erase(vit);
            } else {
                vit++;
            }
        }
    }

    void fuse_players() {
        QSet<int> playerIds;
        for (Player *player : player_array) {
            playerIds.insert(player->getId());
        }

        PlayerArray fused_players;
        for (int id : playerIds) {
            PlayerArray playerFragments = get_players_by_id(id);
            // приведём в предсказуемый порядок
            std::sort(playerFragments.begin(), playerFragments.end(),
                      [](const Player *a, const Player *b) -> bool {
                          if (a->getM() == b->getM()) {
                              return a->get_fId() < b->get_fId();
                          } else {
                              return a->getM() > b->getM();
                          }
                      });
            // перепаковываем в std::list, чтобы не словить UB с итераторами на строчке it2 = fragments.erase(it2);
            std::list<Player*> fragments(playerFragments.begin(), playerFragments.end());
            bool new_fusion_check = true; // проверим всех. Если слияние произошло - перепроверим ещё разок, чтобы все могли слиться в один тик
            while (new_fusion_check) {
                new_fusion_check = false;
                for (auto it = fragments.begin(); it != fragments.end(); ++it) {
                    auto &player = *it;
                    for (auto it2 = std::next(it); it2 != fragments.end(); ) {
                        auto &frag = *it2;
                        if (player->can_fuse(frag)) {
                            player->fusion(frag);
                            fused_players.push_back(frag);
                            new_fusion_check = true;
                            it2 = fragments.erase(it2);
                        } else {
                            ++it2;
                        }
                    }
                }
                if (new_fusion_check) {
                    for (auto it = fragments.begin(); it != fragments.end(); ++it) {
                        bool changed = (*it)->update_by_mass(Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT); // need for future fusing
                        if (changed) {
                            logger->write_change_mass(tick, *it);
                        }
                    }
                }
            }
            if (fragments.size() == 1) {
                Player *player = fragments.front();
                QString old_id = player->id_to_str();
                bool changed = player->clear_fragments();
                if (changed) {
                    logger->write_change_id(tick, old_id, player);
                }
                continue;
            }
        }
        for (Player *p : fused_players) {
            logger->write_kill_cmd(tick, p);
            delete p;
            player_array.removeAll(p);
        }
    }

    void move_moveables() {
        Constants &ins = Constants::instance();
        for (Ejection *eject : eject_array) {
            bool changed = eject->move(ins.GAME_WIDTH, ins.GAME_HEIGHT);
            if (changed) {
                logger->write_change_pos(tick, eject);
            }
        }
        for (Virus *virus : virus_array) {
            bool changed = virus->move(ins.GAME_WIDTH, ins.GAME_HEIGHT);
            if (changed) {
                logger->write_change_pos(tick, virus);
            }
        }

        QSet<int> playerIds;
        for (Player *player : player_array) {
            playerIds.insert(player->getId());
        }

        for (auto sId : playerIds) {
            PlayerArray fragments = get_players_by_id(sId);
            for (int i = 0; i != fragments.size(); ++i) {
                Player *curr = fragments[i];
                for (int j = i + 1; j < fragments.size(); ++j) {
                    curr->collisionCalc(fragments[j]);
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

    void update_players_radius() {
        for (Player *player : player_array) {
            bool changed = player->update_by_mass(Constants::instance().GAME_WIDTH, Constants::instance().GAME_HEIGHT);
            if (changed) {
                logger->write_change_mass(tick, player);
            }
        }
    }

    void split_viruses() {
        VirusArray append_viruses;
        for (Virus *virus : virus_array) {
            if (virus->can_split()) {
                Virus *new_virus = virus->split_now(id_counter);
                logger->write_add_cmd(tick, new_virus);
                append_viruses.append(new_virus);
                id_counter++;
            }
        }
        virus_array.append(append_viruses);
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
