#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QVector>
#include <QDebug>
#include <qmath.h>
#include <QTime>
#include <qglobal.h>
#include <QProcessEnvironment>
#include <random>
#include <QSettings>
#include <QJsonObject>

// yes ugly
#define DEFINE_QSETTINGS(VARIABLE_NAME) QSettings VARIABLE_NAME("LocalRunner.ini", QSettings::IniFormat)

struct Constants
{
private:
    Constants() {}
    ~Constants() {}

    Constants(Constants const&) = delete;
    Constants& operator= (Constants const&) = delete;

public:
    // name                     // default

    int GAME_TICKS;             // 75000 ticks
    int GAME_WIDTH;             // 660
    int GAME_HEIGHT;            // 660
    int SUM_RESP_TIMEOUT;       // 500 secs
    int RESP_TIMEOUT;           // 5 sec

    int TICK_MS;                // 16 ms
    int BASE_TICK;              // every 50 ticks
    std::string SEED;           // from std::random_device

    double INERTION_FACTOR;     // 10.0
    double VISCOSITY;           // 0.25
    double SPEED_FACTOR;        // 25.0

    double FOOD_MASS;           // 1.0
    double VIRUS_RADIUS;        // 22.0
    double VIRUS_SPLIT_MASS;    // 80.0
    int MAX_FRAGS_CNT;          // 10
    int TICKS_TIL_FUSION;       // 250 ticks

    static Constants &instance() {
        static Constants ins;
        return ins;
    }

    static std::string generate_seed(uint length = 10) {
        std::random_device dev;
        const std::string alphabet("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567");
        std::uniform_int_distribution<> dist(0, static_cast<int>(alphabet.length() - 1));

        std::string seed;
        while (seed.length() < length) {
            seed += alphabet[static_cast<uint>(dist(dev))];
        }

        return seed;
    }

    static Constants &initialize(const QProcessEnvironment &env) {
        Constants& c = instance();

        DEFINE_QSETTINGS(settings);
        settings.beginGroup("constants");

#define SET_CONSTANT(NAME, DEFAULT, CONVERT) do {                              \
            c.NAME = getSettingValue(#NAME, env, settings, DEFAULT).CONVERT(); \
        } while(false)

        SET_CONSTANT(GAME_TICKS, "75000", toInt);
#if defined LOCAL_RUNNER

        SET_CONSTANT(GAME_WIDTH, "660", toInt);
        SET_CONSTANT(GAME_HEIGHT, "660", toInt);
        SET_CONSTANT(SUM_RESP_TIMEOUT, "500", toInt);
#elif defined SERVER_RUNNER
        SET_CONSTANT(GAME_WIDTH, "990", toInt);
        SET_CONSTANT(GAME_HEIGHT, "990", toInt);
        SET_CONSTANT(SUM_RESP_TIMEOUT, "150", toInt);
#endif

        SET_CONSTANT(TICK_MS, "16", toInt);
        SET_CONSTANT(BASE_TICK, "50", toInt);
        SET_CONSTANT(INERTION_FACTOR, "10.0", toDouble);
        SET_CONSTANT(VISCOSITY, "0.25", toDouble);
        SET_CONSTANT(SPEED_FACTOR, "25.0", toDouble);
        SET_CONSTANT(FOOD_MASS, "1.0", toDouble);
        SET_CONSTANT(VIRUS_RADIUS, "22.0", toDouble);
        SET_CONSTANT(VIRUS_SPLIT_MASS, "80.0", toDouble);
        SET_CONSTANT(MAX_FRAGS_CNT, "10", toInt);
        SET_CONSTANT(TICKS_TIL_FUSION, "250", toInt);
        SET_CONSTANT(RESP_TIMEOUT, "5", toInt);

#undef SET_CONSTANT

        settings.endGroup();
        settings.sync();

        c.SEED = env.value("SEED", "").toStdString();

        return c;
    }

    QJsonObject toJson() const {
        return {
            {"GAME_WIDTH", GAME_WIDTH},
            {"GAME_HEIGHT", GAME_HEIGHT},
            {"GAME_TICKS", GAME_TICKS},

            {"FOOD_MASS", FOOD_MASS},
            {"MAX_FRAGS_CNT", MAX_FRAGS_CNT},
            {"TICKS_TIL_FUSION", TICKS_TIL_FUSION},
            {"VIRUS_RADIUS", VIRUS_RADIUS},
            {"VIRUS_SPLIT_MASS", VIRUS_SPLIT_MASS},

            {"VISCOSITY", VISCOSITY},
            {"INERTION_FACTOR", INERTION_FACTOR},
            {"SPEED_FACTOR", SPEED_FACTOR},
        };
    }

private:
    static QString getSettingValue(const QString& name,
                                   const QProcessEnvironment& env,
                                   QSettings& section,
                                   const QString& defaultValue)
    {
        if (!section.contains(name)) {
            section.setValue(name, defaultValue);
        }
        return env.value(name, section.value(name).toString());
    }
};

const QString LOG_DIR = "/var/tmp/";
const QString LOG_FILE = "visio_{1}.log";
const QString DEBUG_FILE = "{1}.log";
const QString DUMP_FILE = "{1}_dump.log";
const QString SCORES_FILE = "scores.json";

const QString MAIN_JSON_KEY = "visio";
const QString DEBUG_JSON_KEY = "debug";
const QString SCORES_JSON_KEY = "scores";

const int START_FOOD_SETS = 4;
const int ADD_FOOD_SETS = 2;
const int ADD_FOOD_DELAY = 40;
const double FOOD_RADIUS = 2.5;
//const double FOOD_MASS = 1.0;

const int START_VIRUS_SETS = 1;
const int ADD_VIRUS_SETS = 1;
const int ADD_VIRUS_DELAY = 1200;
//const double VIRUS_RADIUS = 22.0;
const double VIRUS_MASS = 40.0;

const int START_PLAYER_SETS = 1;
const int START_PLAYER_OFFSET = 400;
const double PLAYER_RADIUS_FACTOR = 2;
const double PLAYER_MASS = 40.0;
const double PLAYER_RADIUS = PLAYER_RADIUS_FACTOR * std::sqrt(PLAYER_MASS);

const double VIS_FACTOR = 4.0; // vision = radius * VF
const double VIS_FACTOR_FR = 2.5; // vision = radius * VFF * qSqrt(fragments.count())
const double VIS_SHIFT = 10.0; // dx = qCos(angle) * VS; dy = qSin(angle) * VS
const double DRAW_SPEED_FACTOR = 14.0;

const double COLLISION_POWER = 20.;
const double MASS_EAT_FACTOR = 1.20; // mass > food.mass * MEF
const double DIAM_EAT_FACTOR = 2./3.; // dist - eject->getR() + (eject->getR() * 2) * DIAM_EAT_FACTOR < radius

const double RAD_HURT_FACTOR = 2./3.; // (radius * RHF + player.radius) > dist
const double MIN_BURST_MASS = 60.0; // MBM * 2 < mass
//const int MAX_FRAGS_CNT = 10;
const double BURST_START_SPEED = 8.0;
const double BURST_ANGLE_SPECTRUM = M_PI; // angle - BAM / 2 + I*BAM / frags_cnt
//const double PLAYER_VISCOSITY = 0.25;

//const int TICKS_TIL_FUSION = 250;

const double MIN_SPLIT_MASS = 120.0; // MSM < mass
const double SPLIT_START_SPEED = 9.0;

const double MIN_EJECT_MASS = 40.0;
const double EJECT_START_SPEED = 8.0;
const double EJECT_RADIUS = 4.0;
const double EJECT_MASS = 15.0;
//const double EJECT_VISCOSITY = 0.25;

//const double VIRUS_VISCOSITY = 0.25;
const double VIRUS_SPLIT_SPEED = 8.0;
//const double VIRUS_SPLIT_MASS = 80.0;

const double MIN_SHRINK_MASS = 100;
const double SHRINK_FACTOR = 0.01; // (-1) * (mass - MSM) * SF
const int SHRINK_EVERY_TICK = 50;
const double BURST_BONUS = 5.0; // mass += BB

const int SCORE_FOR_FOOD = 1;
const int SCORE_FOR_PLAYER = 10;
const int SCORE_FOR_LAST = 100;
const int SCORE_FOR_BURST = 2;
const int SCORE_FOR_EJECT = 0;

// TCP Server
const QString HOST = "0.0.0.0";
const int PORT = 8000;

const int MAX_RESP_LEN = 3000;
const int MAX_DEBUG_LEN = 1000;
const int MAX_ID_LEN = 100;

const int CONNECT_TIMEOUT = 200; // seconds
//const int RESP_TIMEOUT = 5; // seconds
//const int SUM_RESP_TIMEOUT = 150; // seconds
const int PRE_PAUSE = 20; // seconds

const QString CONNECT_EXPIRED = "Ожидание соединения превышено!";
const QString RESP_EXPIRED = "Ожидание ответа превышено!";
const QString SUM_RESP_EXPIRED = "Суммарное ожидание клиента превышено. Клиент будет отключён!";
const QString CLIENT_DISCONNECTED = "Решение отключилось от механики до окончания!";

const int MAX_GAME_FOOD = 2000;
const int MAX_GAME_VIRUS = 20;


#endif // CONSTANTS_H
