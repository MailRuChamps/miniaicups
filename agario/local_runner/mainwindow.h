#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimerEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>

#include "strategymodal.h"

#include "mechanic.h"
#include "ui_mainwindow.h"

namespace Ui {
    class MainWindow;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    Mechanic *mechanic;
    StrategyModal *sm;
    QMessageBox *mbox;

    long T;
    int timerId;
    bool is_paused;
    bool isAutoPauseEachTick;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void init_game();
//    Qt::GlobalColor get_color(PS);
//    Strategy* get_strategy(PS, int);

    void on_error(QString msg) {
        is_paused = true;

        mbox->setStandardButtons(QMessageBox::Close);
        mbox->setText(msg);
        mbox->exec();
    }

    void clear_game() {
        if (timerId > 0) {
            killTimer(timerId);
            pause_game_on();
            timerId = -1;
        }
        ui->txt_ticks->setText("");
        ui->leaders->clear();
        mechanic->clear_objects(false);
        this->update();
    }

    void pause_game_on()
    { is_paused = true;  ui->btn_start->setText("Старт"); }
    void pause_game_off()
    { is_paused = false; ui->btn_start->setText("Пауза"); }

    void finish_game() {
        killTimer(timerId);
        pause_game_on();
        timerId = -1;

        QString text = "Игра завершена";
        mbox->setStandardButtons(QMessageBox::Close);
        mbox->setText(text);
        mbox->exec();

        clear_game();
    }
    void slotShowBotCommand(double, double, bool, bool, const QString &debug);

public:
    void paintEvent(QPaintEvent*) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.translate(ui->viewport->x(), ui->viewport->y());
        painter.fillRect(ui->viewport->rect(), QBrush(Qt::white));
        painter.setClipRect(ui->viewport->rect());

        bool show_speed = ui->cbx_speed->isChecked();
        bool show_cmd = ui->cbx_forces->isChecked();
        bool show_fogs = ui->cbx_fog->isChecked();
        mechanic->paintEvent(painter, show_speed, show_fogs, show_cmd);
    }

    void timerEvent(QTimerEvent *event);

public:
    void mouseMoveEvent(QMouseEvent *event)
    {
        int x = event->x() - ui->viewport->x();
        int y = event->y() - ui->viewport->y();
        mechanic->mouseMoveEvent(x, y);
    }

    void mousePressEvent(QMouseEvent *event)
    {
        int x = event->x() - ui->viewport->x();
        int y = event->y() - ui->viewport->y();
        mechanic->mouseMoveEvent(x, y);
    }

    void keyPressEvent(QKeyEvent *event)
    {
             if (event->key() == Qt::Key_F) pause_game_on();
        else if (event->key() == Qt::Key_H) pause_game_off();
        else mechanic->keyPressEvent(event);
    }

    void show_leaders();
};

#endif // MAINWINDOW_H
