#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimerEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>

#include "constants.h"
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

        if (Constants::instance().SEED.empty()) {
            ui->txt_seed->setText(QString::fromStdString(Constants::generate_seed()));
        } else {
            ui->txt_seed->setText(QString::fromStdString(Constants::instance().SEED));
        }


        connect(ui->btn_start, SIGNAL(pressed()), this, SLOT(init_game()));
        connect(ui->btn_stop, SIGNAL(pressed()), this, SLOT(clear_game()));
        connect(ui->btn_pause, SIGNAL(pressed()), this, SLOT(pause_game()));

        connect(ui->cbx_forces, SIGNAL(stateChanged(int)), this, SLOT(update()));
        connect(ui->cbx_speed, SIGNAL(stateChanged(int)), this, SLOT(update()));
        connect(ui->cbx_fog, SIGNAL(stateChanged(int)), this, SLOT(update()));

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

        std::string seed = ui->txt_seed->text().toStdString();

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
        this->update();
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
        this->update();
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

    void timerEvent(QTimerEvent *event) {
        if (event->timerId() == timerId && !is_paused) {
            int tick = mechanic->tickEvent();
            ui->txt_ticks->setText(QString::number(tick));
            this->update();

            if (tick % Constants::instance().BASE_TICK == 0 && tick != 0) {
                show_leaders();
            }
            if (tick % Constants::instance().GAME_TICKS == 0 && tick != 0) {
                finish_game();
            }
        }
    }

public:
    void mouseMoveEvent(QMouseEvent *event) {
        int x = event->x() - ui->viewport->x();
        int y = event->y() - ui->viewport->y();
        mechanic->mouseMoveEvent(x, y);
    }

    void mousePressEvent(QMouseEvent *event) {
        int x = event->x() - ui->viewport->x();
        int y = event->y() - ui->viewport->y();
        mechanic->mouseMoveEvent(x, y);
    }

    void keyPressEvent(QKeyEvent *event) {
        mechanic->keyPressEvent(event);
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
