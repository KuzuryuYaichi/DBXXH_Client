#ifndef CHARTVIEWSPECTRUMNB_H
#define CHARTVIEWSPECTRUMNB_H

#include "ChartViewSpectrum.h"

#include "fftw-3.3.5-dll64/fftw3.h"

class ChartViewSpectrumNB: public ChartViewSpectrum
{
    Q_OBJECT
public:
    ChartViewSpectrumNB(QString, double, double, double, double, QWidget* = nullptr);
    void replace(unsigned char* const, fftw_complex*);

protected:
    QCPGraph *GateSeries;
    void UpdateRuler(QMouseEvent*);
    static constexpr double NB_HALF_BANDWIDTH[] =
        { 0.375 / 2 / 1e3, 0.75 / 2 / 1e3, 1.5 / 2 / 1e3, 3.75 / 2 / 1e3, 6 / 2 / 1e3, 15 / 2 / 1e3,
         22.5 / 2 / 1e3, 37.5 / 2 / 1e3, 75 / 2 / 1e3, 125 / 2 / 1e3, 300 / 2 / 1e3, 375 / 2 / 1e3 };
};

#endif // CHARTVIEWSPECTRUMNB_H
