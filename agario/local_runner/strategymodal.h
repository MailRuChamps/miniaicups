#ifndef STRATEGYMODAL_H
#define STRATEGYMODAL_H

#include "strategies/strategy.h"
#include "strategies/bymouse.h"
#include "strategies/custom.h"

#include <QHash>
#include <QSettings>

struct PlayerGui {
    bool rbn_custom;
    bool rbn_comp;
    bool rbn_mouse;
    QString choose_comp;
    QString choose_color;
    QString edit_custom;
};

class StrategyModal
{
protected:
    QHash<int, PlayerGui> gui_of_player;

public:
    explicit StrategyModal()
    {

#define SM_SETUP_PLAYER_GUI(player_id)                          \
        do {                                                    \
            PlayerGui& cur_gui  = gui_of_player[player_id % 4]; \
            cur_gui.rbn_custom  = false;                        \
            cur_gui.rbn_comp    = false;                        \
            cur_gui.choose_comp = "Ближайшая еда";              \
            cur_gui.choose_color= "Красный";                    \
            cur_gui.edit_custom = "";                           \
        } while (false)

        SM_SETUP_PLAYER_GUI(1);
        SM_SETUP_PLAYER_GUI(2);
        SM_SETUP_PLAYER_GUI(3);
        SM_SETUP_PLAYER_GUI(4);

#undef SM_SETUP_PLAYER_GUI

        DEFINE_QSETTINGS(settings);
        if (settings.value("first_time", true).toBool()) {
            // Default settings are already in the form
            settings.setValue("first_time", false);
            save_settings();
            settings.sync();
        }
        restore_settings();
    }

    void restore_settings() {
        DEFINE_QSETTINGS(settings);
        settings.beginReadArray("players");
        for (int player = 1; player <= 4; ++player) {
            settings.setArrayIndex(player - 1);
            PlayerGui& cur_gui = gui_of_player[player % 4];

            QString strategy_type = settings.value("type").toString();
            if (strategy_type == "Custom")
            { cur_gui.rbn_custom = true; }
            else if (strategy_type == "Comp")
            { cur_gui.rbn_comp = true; }

            QString custom_path = settings.value("custom_path").toString();
            cur_gui.edit_custom = custom_path;

            QString builtin_strategy = settings.value("builtin_strategy").toString();
            cur_gui.choose_comp = builtin_strategy;

            QString color = settings.value("color").toString();
            cur_gui.choose_color = color;
        }
        settings.endArray();
    }

    virtual ~StrategyModal() { }

public:
    QString get_color_name(int playerId) const {
        return gui_of_player[playerId % 4].choose_color;
    }

    Qt::GlobalColor get_color(int playerId) const {
        QString color = get_color_name(playerId);
        if (color == "Красный") return Qt::red;
        else if (color == "Зеленый") return Qt::green;
        else if (color == "Синий") return Qt::blue;
        else if (color == "Желтый") return Qt::yellow;
        else if (color == "Фиолетовый") return Qt::magenta;
        else if (color == "Бирюзовый") return Qt::cyan;
        return Qt::black;
    }

    Strategy* get_strategy(int playerId) const {
        const auto& cur_gui = gui_of_player[playerId % 4];
        if (cur_gui.rbn_comp) {
            QString comp = cur_gui.choose_comp;
            if (comp == "Ближайшая еда") { return new Strategy(playerId); }
            return NULL;
        }
        else if (cur_gui.rbn_custom) {
            QString prog_path = cur_gui.edit_custom;
            return new Custom(playerId, prog_path);
        }
        return NULL;
    }

    void save_settings() {
        DEFINE_QSETTINGS(settings);
        settings.beginWriteArray("players");
        for (int player = 1; player <= 4; ++player) {
            PlayerGui& cur_gui = gui_of_player[player % 4];
            settings.setArrayIndex(player - 1);

            settings.setValue("type", "Comp");

            QString custom_path = cur_gui.edit_custom;
            settings.setValue("custom_path", custom_path);

            QString builtin_strategy = cur_gui.choose_comp;
            settings.setValue("builtin_strategy", builtin_strategy);

            QString color = cur_gui.choose_color;
            settings.setValue("color", color);
        }
        settings.endArray();
        settings.beginGroup("valid_params");
            settings.setValue("Comp", "Стратегия предлагаемая организаторами");
            settings.setValue("Custom", "Ваша стратегия, наделённая ИИ");
        settings.endGroup();

    }
};

#endif // STRATEGYMODAL_H
