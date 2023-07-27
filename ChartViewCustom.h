#ifndef CHARTVIEWCUSTOM_H
#define CHARTVIEWCUSTOM_H

#include "global.h"
#include "QCustomPlot/qcustomplot.h"

#include <QTimer>

class ChartViewCustom: public QCustomPlot
{
    Q_OBJECT
public:
    ChartViewCustom(QString, QString, QString, QWidget* = nullptr);
    void xCenterChanged(double, double);
    void yCenterChanged(double, double);
    void setAxisxMin(double);
    void setAxisxMax(double);
    void setAxisyMin(double);
    void setAxisyMax(double);

protected:
    bool ready = true;
    double xMin = MIN_FREQ, xMax = MAX_FREQ;
    double yMin = MIN_AMPL, yMax = MAX_AMPL;

private:
    void UpdateRuler(QMouseEvent *);
    void UpdateTracer(QMouseEvent *);
    QCPItemTracer* tracer;
    QTimer* m_updater;
    bool isPress = false;
    static constexpr int TIME_INTERVAL = 100;
};

#endif // CHARTVIEWCUSTOM_H
