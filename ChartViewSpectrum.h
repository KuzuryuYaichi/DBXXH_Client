#ifndef CHARTVIEWSPECTRUM_H
#define CHARTVIEWSPECTRUM_H

#include "ChartViewCustom.h"
#include <cmath>
#include <numbers>
#include <array>

class ChartViewSpectrum: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewSpectrum(QString, double, double, double, double, QWidget* = nullptr);

protected:
    void rescaleKeyAxis(const QCPRange& range) override;
    void UpdateTracer(QMouseEvent *);
    QCPGraph *SpectrumSeries, *BoundSeries;
    QCPItemTracer *TracerNormal, *tracer;
    bool LeftButtonPress = false;

signals:
    void RecordThresholdSignal(double);
};

#endif // CHARTVIEWSPECTRUM_H
