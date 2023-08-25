#ifndef CHARTVIEWSPECTRUM_H
#define CHARTVIEWSPECTRUM_H

#include "ChartViewCustom.h"
#include "fftw-3.3.5-dll64/fftw3.h"

class ChartViewSpectrum: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewSpectrum(QString, double, double, double, double, QWidget* = nullptr);
    ~ChartViewSpectrum();
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
    static constexpr auto DDC_LEN = 2048;
    static constexpr double NB_HALF_BANDWIDTH[] =
        {0.15 / 2 / 1e3, 0.3 / 2 / 1e3, 0.6 / 2 / 1e3, 1.5 / 2 / 1e3, 2.4 / 2 / 1e3, 6 / 2 / 1e3,
         9 / 2 / 1e3, 15 / 2 / 1e3, 30 / 2 / 1e3, 50 / 2 / 1e3, 120 / 2 / 1e3, 150 / 2 / 1e3};
};

#endif // CHARTVIEWSPECTRUM_H
