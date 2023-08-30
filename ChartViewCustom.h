#ifndef CHARTVIEWCUSTOM_H
#define CHARTVIEWCUSTOM_H

#include "QCustomPlot/qcustomplot.h"
#include "global.h"
#include "StructNetData.h"
#include <QTimer>

class ChartViewCustom: public QCustomPlot
{
    Q_OBJECT
public:
    ChartViewCustom(QString, QString, QString, QWidget* = nullptr, int = REFRESH_INTERVAL);
    void xRangeChanged(const QCPRange&);
    void yRangeChanged(const QCPRange&);

protected:
    bool ready = true;
    bool readyData = true;
    QCPRange xRange = { MIN_FREQ,  MAX_FREQ }, yRange = { MIN_AMPL, MAX_AMPL };
    double ResolveResolution(int, double);
    virtual void rescaleKeyAxis(const QCPRange&);

private:
    QTimer* m_updater;
    static constexpr int REFRESH_INTERVAL = 100;
};

#endif // CHARTVIEWCUSTOM_H
