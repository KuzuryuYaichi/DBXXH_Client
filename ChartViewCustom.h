#ifndef CHARTVIEWCUSTOM_H
#define CHARTVIEWCUSTOM_H

#include "global.h"
#include "QCustomPlot/qcustomplot.h"
#include <QTimer>

class ChartViewCustom: public QCustomPlot
{
    Q_OBJECT
public:
    ChartViewCustom(QString, QString, QString, QWidget* = nullptr, int = REFRESH_INTERVAL);
    void xCenterChanged(double, double);
    void yCenterChanged(double, double);

protected:
    bool ready = true;
    QCPRange xRange = { MIN_FREQ,  MAX_FREQ }, yRange = { MIN_AMPL, MAX_AMPL };

private:
    QTimer* m_updater;
    static constexpr int REFRESH_INTERVAL = 100;
};

#endif // CHARTVIEWCUSTOM_H
