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
    void replace(unsigned char* const);
    void SeriesSelectChanged(bool = false, bool = false);
    void SaveSpectrum(const QString&);

signals:
    void triggerMark(std::vector<std::tuple<bool, double, double>>);
    void triggerTrack(double, double);
    void triggerMeasure(double);

protected:
    void rescaleKeyAxis(const QCPRange& range) override;

private:
    void UpdateTrack(QMouseEvent *);
    void UpdateRuler(QMouseEvent *);
    void UpdateTracer(QMouseEvent *);
    void InitMenu();
    QCPItemTracer *TracerNormal, *TracerMarker[MARKER_NUM], *tracer;
    QCPItemText* TracerText[MARKER_NUM];
    bool LeftButtonPress = false;
    bool MaxKeepSelect = false, MinKeepSelect = false;
    QMenu* menu;
    bool MenuAppear = true;

    void AnalyzeFrame(size_t);
    void AnalyzeMark(double, double, unsigned char*, unsigned int);
    void AnalyzeMeasure(double, double, unsigned int);
    void AnalyzeTrack(double, double, unsigned char*, unsigned int);
    QVector<double> pointsMax, pointsMin;
    QCPGraph *SpectrumSeries, *BoundSeries, *MaxKeepSeries, *MinKeepSeries, *TrackSeries;
    fftw_complex* inR, * outR;
    fftw_plan planR;
    double TrackStartFreq = 0, TrackEndFreq = 0;
signals:
    void thresholdEnterPressedSignal(double);
private:
    static constexpr double NB_HALF_BANDWIDTH[] =
    { 0.375 / 2 / 1e3, 0.75 / 2 / 1e3, 1.5 / 2 / 1e3, 3.75 / 2 / 1e3, 6 / 2 / 1e3, 15 / 2 / 1e3,
      22.5 / 2 / 1e3, 37.5 / 2 / 1e3, 75 / 2 / 1e3, 125 / 2 / 1e3, 300 / 2 / 1e3, 375 / 2 / 1e3 };

    enum DISPLAY {
        NORMAL,
        MARK,
        MEASURE,
        TRACK
    } DisplayState = NORMAL;

    QCPLayoutGrid* MarkElement;
};

#endif // CHARTVIEWSPECTRUM_H
