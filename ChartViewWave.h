#ifndef CHARTVIEWWAVE_H
#define CHARTVIEWWAVE_H

#include "ChartViewCustom.h"

class ChartViewWave: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewWave(QString, double, double, double, double, QWidget* = nullptr);
    void replace(unsigned char* const buf);

protected:
    void UpdateTracer(QMouseEvent *);
    QCPItemTracer* tracer;
    QCPGraph* ISeries, *QSeries;
};

#endif // CHARTVIEWWAVE_H
