#ifndef STRATEGYMODAL_H
#define STRATEGYMODAL_H

#include "strategies/strategy.h"
#include "strategies/bymouse.h"
#include "strategies/custom.h"

#include <QDialog>
#include <QSettings>

namespace Ui {
    class StrategyModal;
}

struct PS
{
    QString  name;
    QString color;
    QString  path;
    QString  type;
};

class StrategyModal : public QDialog
{
    Q_OBJECT
private:
    QSettings *settings;

protected:
    Ui::StrategyModal *ui;

public:
    explicit StrategyModal(QWidget *parent=NULL);
    virtual ~StrategyModal() { if (ui) delete ui; }

    PS p1, p2, p3, p4;

public:
    void createIniSettings();
    void setSettingsToUi();
    void setSettingsToIni();
    void readSettingsFromUi();
    void readSettingsFromIni();

    const QString get_color_name(int);
    const Qt::GlobalColor get_color(int);
    Strategy* get_strategy(int);

private slots:
    void on_pbtn_saveSettings_released();
};

#endif // STRATEGYMODAL_H
