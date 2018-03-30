#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mechanic(new Mechanic),
    sm(new StrategyModal),
    mbox(new QMessageBox),
    timerId(-1),
    is_paused(true)
{
    ui->setupUi(this);
    this->setMouseTracking(true);
    this->setFixedSize(this->geometry().width(), this->geometry().height());

    connect(ui->btn_start, SIGNAL(pressed()), this, SLOT(init_game()));
    connect(ui->btn_start, SIGNAL(pressed()), ui->viewport, SLOT(setFocus()));
    connect(ui->btn_stop,  SIGNAL(pressed()), this, SLOT(clear_game()));
//    connect(ui->btn_pause, SIGNAL(pressed()), this, SLOT(pause_game()));

    connect(ui->cbx_forces, SIGNAL(stateChanged(int)), this, SLOT(update()));
    connect(ui->cbx_speed, SIGNAL(stateChanged(int)), this, SLOT(update()));
    connect(ui->cbx_fog, SIGNAL(stateChanged(int)), this, SLOT(update()));

    connect(ui->pbtn_settings, SIGNAL(pressed()), sm, SLOT(show()));

    ui->pbtn_settings->setVisible(::ini___->value("isSettingsButton" ).toBool());
}

MainWindow::~MainWindow()
{
    if (ui) delete ui;
    if (mechanic) delete mechanic;
//        if (sm) delete sm;
}

void MainWindow::init_game()
{
    if (is_paused) pause_game_off();
    else pause_game_on();
    if (timerId > 0) return;
    isAutoPauseEachTick = ::ini___->value("autoPauseEachTick" ).toBool();

    time(&T);
    ui->txt_seed->setText(QString::number(T));

    sm->readSettingsFromIni();
    timerId = startTimer(Constants::instance().TICK_MS);
    ui->txt_ticks->setText("0");

    int seed = ui->txt_seed->text().toInt();

    mechanic->init_objects(seed, [this] (Player *player) {
        int pId = player->getId();
        Strategy *strategy;
        player->set_color(sm->get_color(pId));
        strategy = sm->get_strategy(pId);
        Custom *custom = dynamic_cast<Custom*>(strategy);
        if (custom != NULL) {
            connect(custom, SIGNAL(error(QString)), this, SLOT(on_error(QString)));
        }
        return strategy;
    });
    this->update();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId && !is_paused)
    {
        int tick = mechanic->tickEvent();
        ui->txt_ticks->setText(QString::number(tick));
        this->update();

        if (tick % Constants::instance().BASE_TICK == 0 && tick != 0) { show_leaders(); }
        if (tick % Constants::instance().GAME_TICKS == 0 && tick != 0) { finish_game(); }

        if (isAutoPauseEachTick) pause_game_on();
    }
}

void MainWindow::show_leaders()
{
    ui->leaders->clear();

    static QList<QPair<int, int>> scores_list;
    scores_list.clear();
    static QMap<int, int> scores; // = mechanic->get_scores();
    scores = mechanic->get_scores();
    for (int player_id : scores.keys()) {
        scores_list.append(QPair<int, int>(player_id, scores[player_id]));
    }
    qSort(scores_list.begin(), scores_list.end(), [] (QPair<int, int> &a, QPair<int, int> &b) {
        return a.second > b.second;
    });

    static QString line;
    for (auto const& pair : scores_list)
    {
        line.clear();
        switch (pair.first)
        {
            case 1 : line = sm->p1.color + ' ' + sm->p1.name + " = " + QString::number(pair.second); break;
            case 2 : line = sm->p2.color + ' ' + sm->p2.name + " = " + QString::number(pair.second); break;
            case 3 : line = sm->p3.color + ' ' + sm->p3.name + " = " + QString::number(pair.second); break;
            case 4 : line = sm->p4.color + ' ' + sm->p4.name + " = " + QString::number(pair.second); break;
            default : qDebug("bad player id number");
        }
        ui->leaders->addItem(line);
    }
    //        ui->leaders->update();
}
