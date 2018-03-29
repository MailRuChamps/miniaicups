#ifndef STRATEGYMODAL_H
#define STRATEGYMODAL_H

#include "strategies/strategy.h"
#include "strategies/bymouse.h"
#include "strategies/custom.h"

#include "ui_strategymodal.h"
#include <QDialog>
#include <QHash>

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

#define SM_SETUP_PLAYER_GUI(player_id)                        \
        do {                                                  \
            PlayerGui& cur_gui = gui_of_player[player_id];    \
            cur_gui.rbn_custom = ui->rbn_custom_##player_id;  \
            cur_gui.rbn_comp = ui->rbn_comp_##player_id;      \
            cur_gui.rbn_mouse = ui->rbn_mouse_##player_id;    \
            cur_gui.choose_comp = ui->cbx_comp_##player_id;   \
            cur_gui.choose_color = ui->cbx_color_##player_id; \
            cur_gui.edit_custom = ui->edt_prog_##player_id;   \
        } while (false)

        SM_SETUP_PLAYER_GUI(1);
        SM_SETUP_PLAYER_GUI(2);
        SM_SETUP_PLAYER_GUI(3);
        SM_SETUP_PLAYER_GUI(4);

#undef SM_SETUP_PLAYER_GUI
    }


    virtual ~StrategyModal() {
        if (ui) delete ui;
    }

public:
    QString get_color_name(int playerId) const {
        return gui_of_player[playerId].choose_color->currentText();
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
        const auto& cur_gui = gui_of_player[playerId];
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
};

#endif // STRATEGYMODAL_H
