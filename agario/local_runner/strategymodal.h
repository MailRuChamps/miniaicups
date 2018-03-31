#ifndef STRATEGYMODAL_H
#define STRATEGYMODAL_H

#include "strategies/strategy.h"
#include "strategies/bymouse.h"
#include "strategies/custom.h"

#include "ui_strategymodal.h"
#include <QDialog>
#include <QHash>
#include <QSettings>

namespace Ui {
    class StrategyModal;
}

struct PlayerGui {
    QRadioButton* rbn_custom;
    QRadioButton* rbn_comp;
    QRadioButton* rbn_mouse;
    QComboBox* choose_comp;
    QComboBox* choose_color;
    QLineEdit* edit_custom;
};

class StrategyModal : public QDialog
{
    Q_OBJECT

protected:
    Ui::StrategyModal *ui;
    QHash<int, PlayerGui> gui_of_player;

public:
    explicit StrategyModal(QWidget *parent=NULL) :
        QDialog(parent),
        ui(new Ui::StrategyModal)
    {
        ui->setupUi(this);

#define SM_SETUP_PLAYER_GUI(player_id)                          \
        do {                                                    \
            PlayerGui& cur_gui = gui_of_player[player_id % 4];  \
            cur_gui.rbn_custom = ui->rbn_custom_##player_id;    \
            cur_gui.rbn_comp = ui->rbn_comp_##player_id;        \
            cur_gui.rbn_mouse = ui->rbn_mouse_##player_id;      \
            cur_gui.choose_comp = ui->cbx_comp_##player_id;     \
            cur_gui.choose_color = ui->cbx_color_##player_id;   \
            cur_gui.edit_custom = ui->edt_prog_##player_id;     \
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
        } else {
            restore_settings();
        }

        connect(this, &QDialog::accepted, this, &StrategyModal::save_settings);
    }

    void restore_settings() {
        DEFINE_QSETTINGS(settings);
        settings.beginReadArray("players");
        for (int player = 1; player <= 4; ++player) {
            settings.setArrayIndex(player - 1);
            PlayerGui& cur_gui = gui_of_player[player % 4];

            QString strategy_type = settings.value("type").toString();
            if (strategy_type == "Custom") {
                cur_gui.rbn_custom->setChecked(true);
            } else if (strategy_type == "Comp") {
                cur_gui.rbn_comp->setChecked(true);
            } else if (strategy_type == "Mouse") {
                cur_gui.rbn_mouse->setChecked(true);
            }

            QString custom_path = settings.value("custom_path").toString();
            cur_gui.edit_custom->setText(custom_path);

            QString builtin_strategy = settings.value("builtin_strategy").toString();
            cur_gui.choose_comp->setCurrentText(builtin_strategy);

            QString color = settings.value("color").toString();
            cur_gui.choose_color->setCurrentText(color);
        }
        settings.endArray();
    }

    virtual ~StrategyModal() {
        if (ui) delete ui;
    }

public:
    QString get_color_name(int playerId) const {
        return gui_of_player[playerId % 4].choose_color->currentText();
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
        if (cur_gui.rbn_comp->isChecked()) {
            QString comp = cur_gui.choose_comp->currentText();
            if (comp == "Ближайшая еда") { return new Strategy(playerId); }
            return NULL;
        }
        else if (cur_gui.rbn_mouse->isChecked()) {
            return new ByMouse(playerId);
        }
        else if (cur_gui.rbn_custom->isChecked()) {
            QString prog_path = cur_gui.edit_custom->text();
            return new Custom(playerId, prog_path);
        }
        return NULL;
    }

public slots:
    void save_settings() {
        DEFINE_QSETTINGS(settings);
        settings.beginWriteArray("players");
        for (int player = 1; player <= 4; ++player) {
            PlayerGui& cur_gui = gui_of_player[player % 4];
            settings.setArrayIndex(player - 1);

            QString strategy_type;
            if (cur_gui.rbn_custom->isChecked()) {
                strategy_type = "Custom";
            } else if (cur_gui.rbn_comp->isChecked()) {
                strategy_type = "Comp";
            } else if (cur_gui.rbn_mouse->isChecked()) {
                strategy_type = "Mouse";
            }
            settings.setValue("type", strategy_type);

            QString custom_path = cur_gui.edit_custom->text();
            settings.setValue("custom_path", custom_path);

            QString builtin_strategy = cur_gui.choose_comp->currentText();
            settings.setValue("builtin_strategy", builtin_strategy);

            QString color = cur_gui.choose_color->currentText();
            settings.setValue("color", color);
        }
        settings.endArray();
    }
};

#endif // STRATEGYMODAL_H
