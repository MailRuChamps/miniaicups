#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimerEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>

#include "strategymodal.h"
#include "mechanic.h"
#include "ui_mainwindow.h"
#include "gamescene.h"

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

    int timerId;
    bool is_paused;

public:
    explicit MainWindow(QWidget *parent = 0) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        mechanic(new Mechanic),
        sm(new StrategyModal),
        mbox(new QMessageBox),
        timerId(-1),
        is_paused(false)
    {
        ui->setupUi(this);
        this->setMouseTracking(true);
        this->setFixedSize(this->geometry().width(), this->geometry().height());

        long T; time(&T);
        ui->txt_seed->setText(QString::number(T));

        const auto& c = Constants::instance();
        auto* scene = new GameScene(ui->gv);
        scene->setSceneRect(0, 0, c.GAME_WIDTH, c.GAME_HEIGHT);
        scene->setMechanic(mechanic);
        ui->gv->setScene(scene);

        connect(ui->btn_start, SIGNAL(pressed()), this, SLOT(init_game()));
        connect(ui->btn_stop, SIGNAL(pressed()), this, SLOT(clear_game()));
        connect(ui->btn_pause, SIGNAL(pressed()), this, SLOT(pause_game()));

        connect(ui->cbx_forces, SIGNAL(toggled(bool)), mechanic, SLOT(setCommandVisible(bool)));
        connect(ui->cbx_speed, SIGNAL(toggled(bool)), mechanic, SLOT(setSpeedVisible(bool)));
        connect(ui->cbx_fog, SIGNAL(toggled(bool)), mechanic, SLOT(setFogsVisible(bool)));
        mechanic->setCommandVisible(ui->cbx_forces->isChecked());
        mechanic->setSpeedVisible(ui->cbx_speed->isChecked());
        mechanic->setFogsVisible(ui->cbx_fog->isChecked());

        connect(ui->action_1, SIGNAL(triggered(bool)), sm, SLOT(show()));
        connect(ui->btn_strategies_settings, &QPushButton::clicked, sm, &QDialog::show);
    }

    ~MainWindow() {
        if (ui) delete ui;
        if (mechanic) delete mechanic;
        if (sm) delete sm;
    }

public slots:
    void init_game() {
        if (is_paused) {
            pause_game();
        }
        if (timerId > 0) return;
        timerId = startTimer(Constants::instance().TICK_MS);
        ui->txt_ticks->setText("0");

        int seed = ui->txt_seed->text().toInt();

        mechanic->init_objects(seed, [this] (Player *player) {
            int pId = player->getId();
            player->set_color(sm->get_color(pId));

            Strategy *strategy = sm->get_strategy(pId);
            Custom *custom = dynamic_cast<Custom*>(strategy);
            if (custom != NULL) {
                connect(custom, SIGNAL(error(QString)), this, SLOT(on_error(QString)));
            }
            return strategy;
        });
    }

    void on_error(QString msg) {
        is_paused = true;

        mbox->setStandardButtons(QMessageBox::Close);
        mbox->setText(msg);
        mbox->exec();
    }

    void clear_game() {
        if (timerId > 0) {
            killTimer(timerId);
            is_paused = false;
            timerId = -1;
        }
        ui->txt_ticks->setText("");
        ui->leaders->clear();
        mechanic->clear_objects(false);
    }

    void pause_game() {
        is_paused = !is_paused;
    }

    void finish_game() {
        killTimer(timerId);
        is_paused = false;
        timerId = -1;

        double max_score = 0;
        double maxId = -1;

        if (maxId != -1) {
            QString text = "Победителем стал ID=" + QString::number(maxId) + " со счетом " + QString::number(max_score);

            mbox->setStandardButtons(QMessageBox::Close);
            mbox->setText(text);
            mbox->exec();
        }
    }

public:
    void timerEvent(QTimerEvent *event) {
        if (event->timerId() == timerId && !is_paused) {
            int tick = mechanic->tickEvent();
            ui->txt_ticks->setText(QString::number(tick));

            if (tick % Constants::instance().BASE_TICK == 0 && tick != 0) {
                show_leaders();
            }
            if (tick % Constants::instance().GAME_TICKS == 0 && tick != 0) {
                finish_game();
            }
        }
    }

    void show_leaders() {
        QList<QPair<int, int>> scores_list;
        QMap<int, int> scores = mechanic->get_scores();
        for (int player_id : scores.keys()) {
            scores_list.append(QPair<int, int>(player_id, scores[player_id]));
        }
        qSort(scores_list.begin(), scores_list.end(), [] (QPair<int, int> &a, QPair<int, int> &b) {
            return a.second > b.second;
        });

        ui->leaders->clear();
        for (auto const& pair : scores_list) {
            QString line = "" + sm->get_color_name(pair.first) + " =" + QString::number(pair.second);
            ui->leaders->addItem(line);
        }
        ui->leaders->update();
    }
};

#endif // MAINWINDOW_H
