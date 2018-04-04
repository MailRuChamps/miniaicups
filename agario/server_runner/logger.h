#ifndef LOGGER_H
#define LOGGER_H

#include "entities/food.h"
#include "entities/virus.h"
#include "entities/player.h"
#include "entities/ejection.h"

#include <QFile>
#include <QByteArray>
#include "zlib.h"

#define CHUNK 16384


template <typename T1>
QString format(const QString &src, T1 num1) {
    QString stackSrc = src;
    return stackSrc.replace("{1}", QString::number(num1, 'g', 16));
}
template <typename T1, typename T2>
QString format(const QString &src, T1 num1, T2 num2) {
    QString stackSrc = src;
    return stackSrc.replace("{1}", QString::number(num1, 'g', 16)).replace("{2}", QString::number(num2, 'g', 16));
}
template <typename T1, typename T2, typename T3>
QString format(const QString &src, T1 num1, T2 num2, T3 num3) {
    QString stackSrc = src;
    return stackSrc.replace("{1}", QString::number(num1, 'g', 16)).replace("{2}", QString::number(num2, 'g', 16)).replace("{3}", QString::number(num3, 'g', 16));
}
template <typename T1, typename T2, typename T3, typename T4>
QString format(const QString &src, T1 num1, T2 num2, T3 num3, T4 num4) {
    QString stackSrc = src;
    return stackSrc.replace("{1}", QString::number(num1, 'g', 16)).replace("{2}", QString::number(num2, 'g', 16))
            .replace("{3}", QString::number(num3, 'g', 16)).replace("{4}", QString::number(num4, 'g', 16));
}
template <typename T1, typename T2, typename T3, typename T4, typename T5>
QString format(const QString &src, T1 num1, T2 num2, T3 num3, T4 num4, T5 num5) {
    QString stackSrc = src;
    return stackSrc.replace("{1}", QString::number(num1, 'g', 16)).replace("{2}", QString::number(num2, 'g', 16))
            .replace("{3}", QString::number(num3, 'g', 16)).replace("{4}", QString::number(num4, 'g', 16))
            .replace("{5}", QString::number(num5, 'g', 16));
}
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
QString format(const QString &src, T1 num1, T2 num2, T3 num3, T4 num4, T5 num5, T6 num6) {
    QString stackSrc = src;
    return stackSrc.replace("{1}", QString::number(num1, 'g', 16)).replace("{2}", QString::number(num2, 'g', 16))
            .replace("{3}", QString::number(num3, 'g', 16)).replace("{4}", QString::number(num4, 'g', 16))
            .replace("{5}", QString::number(num5, 'g', 16)).replace("{6}", QString::number(num6, 'g', 16));
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
QString format(const QString &src, T1 num1, T2 num2, T3 num3, T4 num4, T5 num5, T6 num6, T7 num7, T8 num8) {
    QString stackSrc = src;
    return stackSrc.replace("{1}", QString::number(num1, 'g', 16)).replace("{2}", QString::number(num2, 'g', 16))
            .replace("{3}", QString::number(num3, 'g', 16)).replace("{4}", QString::number(num4, 'g', 16))
            .replace("{5}", QString::number(num5, 'g', 16)).replace("{6}", QString::number(num6, 'g', 16))
            .replace("{7}", QString::number(num7, 'g', 16)).replace("{8}", QString::number(num8, 'g', 16));
}


class Logger : public QObject
{
    Q_OBJECT

private:
    int current_tick;
    QString file_name;
    QString content;
    QFile file;

public:
    explicit Logger() :
        current_tick(0),
        content(""),
        file_name("")
    {}

    virtual ~Logger() {
        if (file.isOpen()) {
            file.close();
        }
    }

    void init_file(QString part, QString basename, bool debug=true) {
//        QString f = (!debug)? LOG_FILE : DEBUG_FILE;
        file_name = basename.replace("{1}", part);
        QString path = LOG_DIR + file_name;
        file.setFileName(path);
        clear_file();
        if (! debug) {
            write_header(part);
        }
    }

    QString get_file_name() const {
        return file_name;
    }

    QString get_path() const {
        return file.fileName();
    }

    void clear_file() {
        if (file.isOpen()) {
            file.close();
        }
        if (file.open(QFile::WriteOnly|QFile::Truncate)) {
            QTextStream f_Stream(&file);
            f_Stream << "";
        }
        content.clear();
        if (file.isOpen()) {
            file.close();
        }
    }

    void write_cmd(int tick, const QString &cmd) {
        if (tick == current_tick) {
            content.append(cmd);
        }
        else {
#ifdef LOCAL_RUNNER
            flush(false);
#endif

            if (tick != 0) {
                content.append(format("\nT{1}\n", tick));
            }
            content.append(cmd);
            current_tick = tick;
        }
    }

    void flush(bool need_compress=true) {
        if (! file.isOpen()) {
            file.open(QFile::Append);
        }
        QTextStream f_Stream(&file);
        f_Stream << content.toUtf8();
        file.close();

        if (need_compress) {
            QByteArray compressed;
            if (compress(content.toUtf8(), compressed)) {
                QFile archive(get_path() + ".gz");
                if (archive.open(QIODevice::WriteOnly)) {
                    archive.write(compressed);
                    archive.close();
                }
            }
        }
        content.clear();
    }

    bool compress(const QByteArray &content, QByteArray &result) {
        const std::string &data = content.toStdString();

        unsigned char out[CHUNK];
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        if (deflateInit2(&strm, -1, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            return false;
        }
        strm.next_in = (unsigned char*)data.c_str();
        strm.avail_in = data.size();
        do {
            int have;
            strm.avail_out = CHUNK;
            strm.next_out = out;
            if (deflate(&strm, Z_FINISH) == Z_STREAM_ERROR) {
                return false;
            }
            have = CHUNK - strm.avail_out;
            result.append((char*)out, have);
        }
        while (strm.avail_out == 0);

        if (deflateEnd(&strm) != Z_OK) {
            return false;
        }
        return true;
    }

    void write_add_cmd(int tick, Food *food) {
        write_cmd(tick, format("AF{1} X{2} Y{3}\n", food->getId(), food->getX(), food->getY()));
    }

    void write_add_cmd(int tick, Virus *virus) {
        write_cmd(tick, format("AV{1} X{2} Y{3}\n", virus->getId(), virus->getX(), virus->getY()));
    }

    void write_add_cmd(int tick, Player *player) {
        QString cmd = "AP" + player->id_to_str() + " X{1} Y{2} R{3} M{4} C{5} F{6}\n";
        write_cmd(tick, format(cmd, player->getX(), player->getY(), player->getR(), player->getM(), player->getC(), player->getVR()));
    }

    void write_add_cmd(int tick, Ejection *eject) {
        write_cmd(tick, format("AE{1} X{2} Y{3}\n", eject->getId(), eject->getX(), eject->getY()));
    }

    void write_direct(int tick, int id, Direct direct) {
        write_cmd(tick, format("C{1} X{2} Y{3}\n", id, direct.x, direct.y));
    }

    void write_direct_for(int tick, Player *player) {
        QString cmd = "C" + player->id_to_str() + " X{1} Y{2}\n";
        QPair<double, double> norm = player->get_direct_norm();
        write_cmd(tick, format(cmd, norm.first, norm.second));
    }

    void write_fog_for(int tick, Player *player) {
        QString cmd = "+P" + player->id_to_str() + " F{1}\n";
        write_cmd(tick, format(cmd, player->getVR()));
    }

    void write_kill_cmd(int tick, Food *food) {
        write_cmd(tick, format("KF{1}\n", food->getId()));
    }

    void write_kill_cmd(int tick, Player *player) {
        QString cmd = "KP" + player->id_to_str() + "\n";
        write_cmd(tick, cmd);
    }

    void write_kill_cmd(int tick, Virus *virus) {
        write_cmd(tick, format("KV{1}\n", virus->getId()));
    }

    void write_kill_cmd(int tick, Ejection *eject) {
        write_cmd(tick, format("KE{1}\n", eject->getId()));
    }

    void write_change_pos(int tick, Player *player) {
        QString cmd = "+P" + player->id_to_str() + " X{1} Y{2} A{3}\n";
        write_cmd(tick, format(cmd, player->getX(), player->getY(), player->getA()));
    }

    void write_change_pos(int tick, Ejection *eject) {
        write_cmd(tick, format("+E{1} X{2} Y{3}\n", eject->getId(), eject->getX(), eject->getY()));
    }

    void write_change_pos(int tick, Virus *virus) {
        write_cmd(tick, format("+V{1} X{2} Y{3}\n", virus->getId(), virus->getX(), virus->getY()));
    }

    void write_change_mass(int tick, Player *player) {
        QString cmd = "+P" + player->id_to_str() + " X{1} Y{2} R{3} M{4}\n";
        write_cmd(tick, format(cmd, player->getX(), player->getY(), player->getR(), player->getM()));
    }

    void write_change_mass_id(int tick, const QString &old_id, Player *player) {
        QString cmd = "+P" + old_id + " R{1} M{2} I" + player->id_to_str() + "\n";
        write_cmd(tick, format(cmd, player->getR(), player->getM()));
    }

    void write_change_id(int tick, const QString &old_id, Player *player) {
        QString cmd = "+P" + old_id + " I" + player->id_to_str() + "\n";
        write_cmd(tick, cmd);
    }

    inline QString escape(QString src) {
        return src.replace(QRegExp("\\n"), "\\n").replace(QRegExp("\""), "\\\"");
    }

    void write_debug(int tick, int pId, const QString &msg) {
        write_cmd(tick, format("D{1} M\"{2}\"\n", pId).replace("{2}", escape(msg)));
    }

    void write_to_sprite(int tick, int pId, const QString &playerId, const QString &msg) {
        QString cmd = format("S{1} I{2} M\"{3}\"\n", pId);
        write_cmd(tick, cmd.replace("{2}", playerId).replace("{3}", escape(msg)));
    }

    void write_error(int tick, int pId, const QString &error) {
        write_cmd(tick, format("E{1} M\"{2}\"\n", pId).replace("{2}", escape(error)));
    }

    void write_error(int pId, const QString &error) {
        write_cmd(current_tick, format("E{1} M\"{2}\"\n", pId).replace("{2}", escape(error)));
    }

    void write_solution_id(int pId, const QString &solution_id) {
        write_cmd(current_tick, format("OI{1} S{2}\n", pId).replace("{2}", solution_id));
    }

    void write_player_score(int tick, int pId, int score) {
        write_cmd(tick, format("P{1} C{2}\n", pId, score));
    }

    void rewrite_game_ticks(int ticks) {
        QString oldLine = format("OD T{1} G{2} B{3}\n", Constants::instance().TICK_MS, Constants::instance().GAME_TICKS, Constants::instance().BASE_TICK);
        QString newLine = format("OD T{1} G{2} B{3}\n", Constants::instance().TICK_MS, ticks, Constants::instance().BASE_TICK);
        content.replace(oldLine, newLine);
    }

    void write_raw(int tick, QString raw) {
        write_cmd(tick, raw);
    }

    void write_raw_with_old_tick(QString raw) {
        write_cmd(current_tick, raw);
    }

private:
    void write_header(const QString &seed) {
        Constants &ins = Constants::instance();
        write_cmd(0, "# O=Options, A=Add, +=Change K=Kill, C=Command, T=Tick, W=World, F=Food, P=Player, V=Virus, E=Ejection\n");
        write_cmd(0, format("# Dynamic params VISCOSITY={1} FOOD_MASS={2} MAX_FRAGS_CNT={3} TICKS_TIL_FUSION={4} INERTION_FACTOR={5} VIRUS_SPLIT_MASS={6} SPEED_FACTOR={7} VIRUS_RADIUS={8}\n",
                            ins.VISCOSITY, ins.FOOD_MASS, ins.MAX_FRAGS_CNT, ins.TICKS_TIL_FUSION, ins.INERTION_FACTOR, ins.VIRUS_SPLIT_MASS, ins.SPEED_FACTOR, ins.VIRUS_RADIUS));
        write_cmd(0, format("OD T{1} G{2} B{3}\n", ins.TICK_MS, ins.GAME_TICKS, ins.BASE_TICK));
        write_cmd(0, format("OW W{1} H{2} S{3}\n", ins.GAME_WIDTH, ins.GAME_HEIGHT).replace("{3}", seed));
        write_cmd(0, format("OF R{1} M{2}\n", FOOD_RADIUS, ins.FOOD_MASS));
        write_cmd(0, format("OV R{1} M{2}\n", ins.VIRUS_RADIUS, VIRUS_MASS));
        write_cmd(0, format("OP R{1} M{2}\n", PLAYER_RADIUS, PLAYER_MASS));
        write_cmd(0, format("OE R{1} M{2}\n", EJECT_RADIUS, EJECT_MASS));
        write_cmd(0, format("OFog S{1}\n", VIS_SHIFT));
    }
};

#endif // LOGGER_H
