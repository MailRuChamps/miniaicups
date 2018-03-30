#include "strategymodal.h"


#include "ui_strategymodal.h"

StrategyModal::StrategyModal(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::StrategyModal)
{
    ui->setupUi(this);

    QStringList strList;
    strList << "Красный" << "Зеленый" << "Синий" << "Желтый" << "Фиолетовый" << "Бирюзовый";
    ui->cbx_color_1->addItems(strList); ui->cbx_color_1->setCurrentIndex(0);
    ui->cbx_color_2->addItems(strList); ui->cbx_color_2->setCurrentIndex(1);
    ui->cbx_color_3->addItems(strList); ui->cbx_color_3->setCurrentIndex(2);
    ui->cbx_color_4->addItems(strList); ui->cbx_color_4->setCurrentIndex(3);

    readSettingsFromIni();
}

void StrategyModal::readSettingsFromIni()
{
    p1. name = ::ini___->value("player1/name" ).toString();
    p1.color = ::ini___->value("player1/color").toString();
    p1. path = ::ini___->value("player1/path" ).toString();
    p1. type = ::ini___->value("player1/type" ).toString();

    p2. name = ::ini___->value("player2/name" ).toString();
    p2.color = ::ini___->value("player2/color").toString();
    p2. path = ::ini___->value("player2/path" ).toString();
    p2. type = ::ini___->value("player2/type" ).toString();

    p3. name = ::ini___->value("player3/name" ).toString();
    p3.color = ::ini___->value("player3/color").toString();
    p3. path = ::ini___->value("player3/path" ).toString();
    p3. type = ::ini___->value("player3/type" ).toString();

    p4. name = ::ini___->value("player4/name" ).toString();
    p4.color = ::ini___->value("player4/color").toString();
    p4. path = ::ini___->value("player4/path" ).toString();
    p4. type = ::ini___->value("player4/type" ).toString();
    setSettingsToUi();
}
void StrategyModal::setSettingsToIni()
{
    ::ini___->setValue("player1/name" , p1.name );
    ::ini___->setValue("player1/color", p1.color);
    ::ini___->setValue("player1/path" , p1.path );
    ::ini___->setValue("player1/type" , p1.type );

    ::ini___->setValue("player2/name" , p2.name );
    ::ini___->setValue("player2/color", p2.color);
    ::ini___->setValue("player2/path" , p2.path );
    ::ini___->setValue("player2/type" , p2.type );

    ::ini___->setValue("player3/name" , p3.name );
    ::ini___->setValue("player3/color", p3.color);
    ::ini___->setValue("player3/path" , p3.path );
    ::ini___->setValue("player3/type" , p3.type );

    ::ini___->setValue("player4/name" , p4.name );
    ::ini___->setValue("player4/color", p4.color);
    ::ini___->setValue("player4/path" , p4.path );
    ::ini___->setValue("player4/type" , p4.type );

    ::ini___->sync();
}
void StrategyModal::setSettingsToUi()
{
    ui->edt_name_1->setText(p1.name);
    ui->cbx_color_1->setCurrentIndex(ui->cbx_color_1->findText(p1.color));
    ui->edt_prog_1->setText(p1.path);
         if (p1.type == "PC"   ) ui->rbn_comp_1  ->setChecked(true);
    else if (p1.type == "local") ui->rbn_custom_1->setChecked(true);
    else if (p1.type == "mouse") ui->rbn_mouse_1 ->setChecked(true);

    ui->edt_name_2->setText(p2.name);
    ui->cbx_color_2->setCurrentIndex(ui->cbx_color_2->findText(p2.color));
    ui->edt_prog_2->setText(p2.path);
         if (p2.type == "PC"   ) ui->rbn_comp_2  ->setChecked(true);
    else if (p2.type == "local") ui->rbn_custom_2->setChecked(true);
    else if (p2.type == "mouse") ui->rbn_mouse_2 ->setChecked(true);

    ui->edt_name_3->setText(p3.name);
    ui->cbx_color_3->setCurrentIndex(ui->cbx_color_3->findText(p3.color));
    ui->edt_prog_3->setText(p3.path);
         if (p3.type == "PC"   ) ui->rbn_comp_3  ->setChecked(true);
    else if (p3.type == "local") ui->rbn_custom_3->setChecked(true);
    else if (p3.type == "mouse") ui->rbn_mouse_3 ->setChecked(true);

    ui->edt_name_4->setText(p4.name);
    ui->cbx_color_4->setCurrentIndex(ui->cbx_color_4->findText(p4.color));
    ui->edt_prog_4->setText(p4.path);
         if (p4.type == "PC"   ) ui->rbn_comp_4  ->setChecked(true);
    else if (p4.type == "local") ui->rbn_custom_4->setChecked(true);
    else if (p4.type == "mouse") ui->rbn_mouse_4 ->setChecked(true);
}
void StrategyModal::readSettingsFromUi()
{
    p1.color = ui->cbx_color_1->currentText();
    p1.name = ui->edt_name_1->text();
    p1.path = ui->edt_prog_1->text();
         if (ui->rbn_comp_1->isChecked())  p1.type = "PC";
    else if (ui->rbn_custom_1->isChecked())p1.type = "local";
    else if (ui->rbn_mouse_1->isChecked()) p1.type = "mouse";

    p2.color = ui->cbx_color_2->currentText();
    p2.name = ui->edt_name_2->text();
    p2.path = ui->edt_prog_2->text();
         if (ui->rbn_comp_2->isChecked())  p2.type = "PC";
    else if (ui->rbn_custom_2->isChecked())p2.type = "local";
    else if (ui->rbn_mouse_2->isChecked()) p2.type = "mouse";

    p3.color = ui->cbx_color_3->currentText();
    p3.name = ui->edt_name_3->text();
    p3.path = ui->edt_prog_3->text();
         if (ui->rbn_comp_3->isChecked())  p3.type = "PC";
    else if (ui->rbn_custom_3->isChecked())p3.type = "local";
    else if (ui->rbn_mouse_3->isChecked()) p3.type = "mouse";

    p4.color = ui->cbx_color_4->currentText();
    p4.name = ui->edt_name_4->text();
    p4.path = ui->edt_prog_4->text();
         if (ui->rbn_comp_4->isChecked())  p4.type = "PC";
    else if (ui->rbn_custom_4->isChecked())p4.type = "local";
    else if (ui->rbn_mouse_4->isChecked()) p4.type = "mouse";
}

const QString StrategyModal::get_color_name(int playerId)
{
    switch (playerId)
    {
        case 1 : return ui->cbx_color_1->currentText(); break;
        case 2 : return ui->cbx_color_2->currentText(); break;
        case 3 : return ui->cbx_color_3->currentText(); break;
        case 4 : return ui->cbx_color_4->currentText(); break;
        default : return "";
    }
}
const Qt::GlobalColor StrategyModal::get_color(int playerId)
{
    QString color = get_color_name(playerId);
         if (color == "Красный"   ) return Qt::red;
    else if (color == "Зеленый"   ) return Qt::green;
    else if (color == "Синий"     ) return Qt::blue;
    else if (color == "Желтый"    ) return Qt::yellow;
    else if (color == "Фиолетовый") return Qt::magenta;
    else if (color == "Бирюзовый" ) return Qt::cyan;
    return Qt::black;
}
Strategy* StrategyModal::get_strategy(int playerId)
{
    PS ps;
    switch (playerId)
    {
        case 1 : ps = p1; break;
        case 2 : ps = p2; break;
        case 3 : ps = p3; break;
        case 4 : ps = p4; break;
        default : return NULL;
    }
         if (ps.type == "PC"   ) { return new Strategy(playerId);        }
    else if (ps.type == "mouse") { return new ByMouse(playerId);         }
    else if (ps.type == "local") { return new Custom(playerId, ps.path); }
    return NULL;
}

void StrategyModal::on_pbtn_saveSettings_released()
{
    readSettingsFromUi();
    setSettingsToIni();
}
