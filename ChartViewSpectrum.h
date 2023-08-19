#ifndef CHARTVIEWSPECTRUM_H
#define CHARTVIEWSPECTRUM_H

#include "ChartViewCustom.h"
#include "fftw-3.3.5-dll64/fftw3.h"

class ChartViewSpectrum: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewSpectrum(QString, QString, int, int, QString, int, int, QWidget* = nullptr);
    void replace(unsigned char* const buf);
    QCPTextElement* thresholdLbl;

private:
    void UpdateRuler(QMouseEvent *);
    void UpdateTracer(QMouseEvent *);
    QCPItemTracer* tracer;
    bool isPress = false;

    void analyzeFrame(unsigned char*, size_t);
    QVector<double> pointsMax, pointsMin;
    QCPGraph *SpectrumSeries, *GateSeries, *MaxKeepSeries, *MinKeepSeries;
    fftw_complex* inR, * outR;
    fftw_plan planR;
signals:
    void thresholdEnterPressedSignal(double);
private:
    static constexpr double NB_HALF_BANDWIDTH[] = {3.90625 / 2 / 1e3, 7.8125 / 2 / 1e3, 15.625 / 2 / 1e3, 31.25 / 2 / 1e3, 62.5 / 2 / 1e3, 125 / 2 / 1e3, 156.25 / 2 / 1e3};
};

#endif // CHARTVIEWSPECTRUM_H
