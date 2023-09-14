#ifndef CHARTVIEWSPECTRUM_H
#define CHARTVIEWSPECTRUM_H

#include "ChartViewCustom.h"
#include "fftw-3.3.5-dll64/fftw3.h"
#include <cmath>
#include <numbers>
#include <array>

template<int N>
constexpr auto HanningWindow()
{
    std::array<double, N> WINDOW;
    for (int i = 0; i < N; i++)
    {
        WINDOW[i] = 0.5 * (1 - std::cos(2 * std::numbers::pi * (i + 1) / (N + 1)));
    }
    return WINDOW;
}

class ChartViewSpectrum: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewSpectrum(QString, double, double, double, double, QWidget* = nullptr);
    ~ChartViewSpectrum();
    void replace(unsigned char* const buf);
    void SeriesSelectChanged(bool MaxKeepSelect = true, bool MinKeepBox = true, bool SpectrumBox = true);

protected:
    void rescaleKeyAxis(const QCPRange& range) override;

private:
    void UpdateRect(QMouseEvent *);
    void UpdateRuler(QMouseEvent *);
    void UpdateTracer(QMouseEvent *);
    void InitMenu();
    QCPItemTracer *TracerNormal, *TracerMarker[MARKER_NUM], *tracer;
    bool LeftButtonPress = false;
    bool MaxKeepSelect = true, MinKeepSelect = true, SpectrumSelect = true;
    QMenu* menu;
    bool MenuAppear = true;

    void analyzeFrame(size_t);
    QVector<double> pointsMax, pointsMin;
    QCPGraph *SpectrumSeries, *BoundSeries, *MaxKeepSeries, *MinKeepSeries, *RectSeries;
    fftw_complex* inR, * outR;
    fftw_plan planR;
    double RectStartValue;
signals:
    void thresholdEnterPressedSignal(double);
private:
    static constexpr double NB_HALF_BANDWIDTH[] =
        {0.15 / 2 / 1e3, 0.3 / 2 / 1e3, 0.6 / 2 / 1e3, 1.5 / 2 / 1e3, 2.4 / 2 / 1e3, 6 / 2 / 1e3,
         9 / 2 / 1e3, 15 / 2 / 1e3, 30 / 2 / 1e3, 50 / 2 / 1e3, 120 / 2 / 1e3, 150 / 2 / 1e3};

    enum DISPLAY {
        NORMAL,
        MARK,
        MEASURE,
        TRACK
    } DisplayState = NORMAL;
};

#endif // CHARTVIEWSPECTRUM_H
