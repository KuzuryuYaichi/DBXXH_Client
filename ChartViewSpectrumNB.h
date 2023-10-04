#ifndef CHARTVIEWSPECTRUMNB_H
#define CHARTVIEWSPECTRUMNB_H

#include "ChartViewSpectrum.h"

class ChartViewSpectrumNB: public ChartViewSpectrum
{
    Q_OBJECT
public:
    ChartViewSpectrumNB(QString, double, double, double, double, QWidget* = nullptr);
    void replace(unsigned char* const, unsigned char*);

protected:
    QCPGraph *GateSeries;
    QCPTextElement* ThresholdLbl;
    void UpdateRuler(QMouseEvent*);
    void UpdateThreshold();

signals:
    void RecordThresholdSignal(double);
};

#endif // CHARTVIEWSPECTRUMNB_H
