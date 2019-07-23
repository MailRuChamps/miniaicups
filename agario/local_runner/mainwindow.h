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

    QMap<int, bool> player_vision;
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

        ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
        ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableWidget->setFocusPolicy(Qt::NoFocus);
        ui->tableWidget->resizeColumnsToContents();
        ui->tableWidget->resizeRowsToContents();
        ui->tableWidget->setSortingEnabled(true);
        ui->tableWidget->sortByColumn(2, Qt::DescendingOrder);
        ui->tableWidget->horizontalHeader()->setSortIndicatorShown(true);
        ui->tableWidget->verticalHeader()->hide();

        ui->txt_ticks->setText("0");
        connect(ui->btn_start_pause, SIGNAL(pressed()), this, SLOT(init_game()));
        connect(ui->btn_stop, SIGNAL(pressed()), this, SLOT(clear_game()));

        connect(ui->cbx_forces, SIGNAL(stateChanged(int)), this, SLOT(update()));
        connect(ui->cbx_speed, SIGNAL(stateChanged(int)), this, SLOT(update()));
        connect(ui->cbx_fog, SIGNAL(stateChanged(int)), this, SLOT(update()));
        connect(ui->cbx_notimeout, SIGNAL(stateChanged(int)), this, SLOT(update_notimeout()));

        connect(ui->btn_strategies_settings, &QPushButton::clicked, sm, &QDialog::show);

        ui->btn_newSeed->setVisible(false);
    }

    ~MainWindow() {
        if (ui) delete ui;
        if (mechanic) delete mechanic;
        if (sm) delete sm;
    }



public slots:
    void init_game() {
        if (timerId > 0) {
            pause_game();
            return;
        }
        timerId = startTimer(Constants::instance().TICK_MS);

        std::string seed = ui->txt_seed->text().toStdString();
        ui->btn_start_pause->setText("Пауза");

        ui->tableWidget->setSortingEnabled(false);
        ui->tableWidget->setRowCount(0);

        mechanic->init_objects(seed, [this] (Player *player) {
            int pId = player->getId();
            player->set_color(sm->get_color(pId));

            Strategy *strategy = sm->get_strategy(pId);
            Custom *custom = dynamic_cast<Custom*>(strategy);
            if (custom != NULL) {
                connect(custom, SIGNAL(error(QString)), this, SLOT(on_error(QString)));
            }

            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);

            int col = 0;

            ui->tableWidget->setItem(row, col++, new QTableWidgetItem(QString::number(pId)));
            ui->tableWidget->setItem(row, col++, new QTableWidgetItem(sm->get_color_name(pId)));
            ui->tableWidget->setItem(row, col++, new QTableWidgetItem("0"));

            //centered checkbox
            QWidget *widget = new QWidget(this);
            QCheckBox *checkBox = new QCheckBox(widget);
            QHBoxLayout *layout = new QHBoxLayout(widget);
            layout->addWidget(checkBox);
            layout->setAlignment(Qt::AlignCenter);
            layout->setContentsMargins(0,0,0,0);
            widget->setLayout(layout);
            checkBox->setProperty("pId", pId);
            checkBox->setChecked(player_vision.value(pId));
            connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(set_vision(bool)));
            ui->tableWidget->setCellWidget(row, col++, widget);

            return strategy;
        });

        for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
            for (int col = 0; col < ui->tableWidget->columnCount() - 1; ++col)
                ui->tableWidget->item(row, col)->setTextAlignment(Qt::AlignCenter);


        ui->tableWidget->resizeColumnsToContents();
        ui->tableWidget->resizeRowsToContents();
        ui->tableWidget->setSortingEnabled(true);

        this->update();
    }

    void on_error(QString msg) {
        is_paused = true;

        mbox->setStandardButtons(QMessageBox::Close);
        mbox->setText(msg);
        mbox->exec();
    }

    void set_vision(bool b) {
        auto cb = (QCheckBox*)QObject::sender();
        auto pid = cb->property("pId").toInt();
        player_vision[pid] = b;
        update();
    }

    void clear_game() {
        if (timerId > 0) {
            killTimer(timerId);
            is_paused = false;
            timerId = -1;
        }
        ui->txt_ticks->setText("");
        mechanic->clear_objects(false);
        ui->btn_start_pause->setText("Старт");
        this->update();
    }

    void update_notimeout() {
        static int originalTimeout;
        static int originalSumTimeout;
        auto &c = Constants::instance();
        if (ui->cbx_notimeout->isChecked()) {
            originalTimeout = c.RESP_TIMEOUT;
            originalSumTimeout = c.SUM_RESP_TIMEOUT;
            c.RESP_TIMEOUT = 999999;
            c.SUM_RESP_TIMEOUT = 999999;
        } else {
            // assuming static values already filled by previous call
            c.RESP_TIMEOUT = originalTimeout;
            c.SUM_RESP_TIMEOUT = originalSumTimeout;
        }
    }

    void pause_game() {
        is_paused = !is_paused;
        if (is_paused) ui->btn_start_pause->setText("Продолжить");
        else ui->btn_start_pause->setText("Пауза");
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

        mechanic->paintEvent(painter, show_speed, show_fogs, show_cmd, player_vision);
    }

    void timerEvent(QTimerEvent *event) {
        if (event->timerId() == timerId && !is_paused) {
            int tick = mechanic->tickEvent();
            ui->txt_ticks->setText(QString::number(tick));
            this->update();

            if (tick % Constants::instance().BASE_TICK == 0 && tick != 0) {
                update_score();
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

    void update_score() {
        QMap<int, int> scores = mechanic->get_scores();

        ui->tableWidget->setSortingEnabled(false);

        for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
            int pId = ui->tableWidget->item(row, 0)->data(Qt::DisplayRole).toInt();
            ui->tableWidget->item(row, 2)->setData(Qt::DisplayRole, scores.value(pId));
        }

         ui->tableWidget->setSortingEnabled(true);
     }
};

#endif // MAINWINDOW_H
