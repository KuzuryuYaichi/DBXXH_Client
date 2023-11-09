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
    void UpdateTracer(QMouseEvent *);
    QCPTextElement* ThresholdLbl;
    QCPGraph *SpectrumSeries, *BandwidthSeries, *GateSeries;
    QCPItemTracer *TracerNormal;
    bool LeftButtonPress = false;
};

#endif // CHARTVIEWSPECTRUM_H
