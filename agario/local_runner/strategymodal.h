#ifndef STRATEGYMODAL_H
#define STRATEGYMODAL_H

#include "strategies/strategy.h"
#include "strategies/bymouse.h"
#include "strategies/custom.h"

#include "ui_strategymodal.h"
#include <QDialog>
#include "config.h"
#include <string>
#include <array>
#include <tuple>

namespace Ui {
    class StrategyModal;
}

class StrategyModal : public QDialog
{
    Q_OBJECT

protected:
    Ui::StrategyModal *ui;
    Config& config;
public:
    explicit StrategyModal(Config& config, QWidget *parent=NULL) :
        QDialog(parent),
        ui(new Ui::StrategyModal),
        config(config)
    {
        ui->setupUi(this);
        connect(ui->pushButton, SIGNAL(pressed()), this, SLOT(saveProperties()));
        loadProperties();
    }

    virtual ~StrategyModal() {
        if (ui) delete ui;
    }
public slots:
    inline void saveProperties() {
        using namespace std;
        auto botGuiProperties = getBotGuiProperties();
        auto guiProps = botGuiProperties.begin();

        for (const auto& groupName: playerGroupNames()) {
            const auto typeSettingPath = QString::fromStdString(groupName + "/TYPE");
            const auto type = config.getString(typeSettingPath, "");

            if (get<3>(*guiProps)->isChecked()) {
                config.setString(typeSettingPath, "USER");
            } else if (get<2>(*guiProps)->isChecked()) {
                config.setString(typeSettingPath, "START_BOT");
            } else if (get<1>(*guiProps)->isChecked()) {
                config.setString(typeSettingPath, "MY_BOT");
                config.setString(QString::fromStdString(groupName + "/PATH"), get<0>(*guiProps)->text());
            }
            ++guiProps;
        }
    }
public:
    inline void loadProperties() {
        using namespace std;
        auto botGuiProperties = getBotGuiProperties();
        auto guiProps = botGuiProperties.begin();

        for (const auto& groupName: playerGroupNames()) {
            const auto typeSettingPath = QString::fromStdString(groupName + "/TYPE");
            const auto type = config.getString(typeSettingPath, "");

            if (type == "USER") {
                get<3>(*guiProps)->setChecked(true);
            } else if (type == "START_BOT") {
                get<2>(*guiProps)->setChecked(true);
            } else if (type == "MY_BOT") {
                get<1>(*guiProps)->setChecked(true);
                get<0>(*guiProps)->setText(
                    config.getString(QString::fromStdString(groupName + "/PATH"), "")
                );
            }
            ++guiProps;
        }
    }

    QString get_color_name(int playerId) const {
        QComboBox *cbx_color;
        if (playerId % 4 == 1) cbx_color = ui->cbx_color_1;
        else if (playerId % 4 == 2) cbx_color = ui->cbx_color_5;
        else if (playerId % 4 == 3) cbx_color = ui->cbx_color_7;
        else if (playerId % 4 == 0) cbx_color = ui->cbx_color_6;
        else return "";
        return cbx_color->currentText();
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
        QRadioButton *rbn_comp, *rbn_custom, *rbn_mouse;
        QComboBox *cbx_comp;
        QLineEdit *edt_prog;

        if (playerId % 4 == 1) {
            rbn_comp = ui->rbn_comp_1; rbn_custom = ui->rbn_custom_1; rbn_mouse = ui->rbn_mouse_1;
            cbx_comp = ui->cbx_comp_1; edt_prog = ui->edt_prog_1;
        }
        else if (playerId % 4 == 2) {
            rbn_comp = ui->rbn_comp_5; rbn_custom = ui->rbn_custom_5; rbn_mouse = ui->rbn_mouse_5;
            cbx_comp = ui->cbx_comp_5; edt_prog = ui->edt_prog_5;
        }
        else if (playerId % 4 == 3) {
            rbn_comp = ui->rbn_comp_7; rbn_custom = ui->rbn_custom_7; rbn_mouse = ui->rbn_mouse_7;
            cbx_comp = ui->cbx_comp_7; edt_prog = ui->edt_prog_7;
        }
        else if (playerId % 4 == 0) {
            rbn_comp = ui->rbn_comp_6; rbn_custom = ui->rbn_custom_6; rbn_mouse = ui->rbn_mouse_6;
            cbx_comp = ui->cbx_comp_6; edt_prog = ui->edt_prog_6;
        }
        else {
            return NULL;
        }

        if (rbn_comp->isChecked()) {
            QString comp = cbx_comp->currentText();
            if (comp == "Ближайшая еда") { return new Strategy(playerId); }
            return NULL;
        }
        else if (rbn_mouse->isChecked()) {
            return new ByMouse(playerId);
        }
        else if (rbn_custom->isChecked()) {
            QString prog_path = edt_prog->text();
            return new Custom(playerId, prog_path);
        }
        return NULL;
    }
private:
    inline std::array<std::string, 4> playerGroupNames() const {
        return { "Player1", "Player2", "Player3", "Player4" };
    }

    inline std::array<std::tuple<QLineEdit*, QRadioButton*, QRadioButton*, QRadioButton*>, 4> getBotGuiProperties() {
        return {
                std::make_tuple(ui->edt_prog_1, ui->rbn_custom_1, ui->rbn_comp_1, ui->rbn_mouse_1),
                std::make_tuple(ui->edt_prog_5, ui->rbn_custom_5, ui->rbn_comp_5, ui->rbn_mouse_5),
                std::make_tuple(ui->edt_prog_7, ui->rbn_custom_7, ui->rbn_comp_7, ui->rbn_mouse_7),
                std::make_tuple(ui->edt_prog_6, ui->rbn_custom_6, ui->rbn_comp_6, ui->rbn_mouse_6)
            };
    };
};

#endif // STRATEGYMODAL_H
