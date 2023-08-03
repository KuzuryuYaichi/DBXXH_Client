#ifndef CHARTVIEWWAVE_H
#define CHARTVIEWWAVE_H

#include "ChartViewCustom.h"

class ChartViewWave: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewWave(QString, QString, int, int, QString, int, int, QWidget* = nullptr);
    void replace(unsigned char* const buf);

private:
    void UpdateTracer(QMouseEvent *);
    QCPItemTracer* tracer;
    QCPGraph* ISeries, *QSeries;
};

#endif // CHARTVIEWWAVE_H