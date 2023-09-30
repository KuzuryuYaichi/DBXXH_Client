#ifndef CHARTVIEWSPECTRUMWB_H
#define CHARTVIEWSPECTRUMWB_H

#include "ChartViewSpectrum.h"

class ChartViewSpectrumWB: public ChartViewSpectrum
{
    Q_OBJECT
public:
    ChartViewSpectrumWB(QString, double, double, double, double, QWidget* = nullptr);
    void SeriesSelectChanged(bool = false, bool = false);
    void SaveSpectrum(const QString&);
    void replace(unsigned char* const);

signals:
    void triggerMark(std::vector<std::tuple<bool, double, double>>);
    void triggerTrack(double, double);
    void triggerMeasure(double);

protected:
    QCPGraph *MaxKeepSeries, *MinKeepSeries, *TrackSeries;
    QCPItemTracer *TracerMarker[MARKER_NUM];
    QCPItemText* TracerText[MARKER_NUM];
    QCPLayoutGrid* MarkElement;
    double TrackStartFreq = 0, TrackEndFreq = 0;
    void AnalyzeFrame(size_t);
    void AnalyzeMark(double, double, unsigned char*, unsigned int);
    void AnalyzeMeasure(double, double, unsigned int);
    void AnalyzeTrack(double, double, unsigned char*, unsigned int);
    QVector<double> pointsMax, pointsMin;
    bool MaxKeepSelect = false, MinKeepSelect = false;
    QMenu* menu;
    bool MenuAppear = true;
    void UpdateTrack(QMouseEvent *);
    void UpdateRuler(QMouseEvent *);
    void InitMenu();
    enum DISPLAY {
        NORMAL,
        MARK,
        MEASURE,
        TRACK
    } DisplayState = NORMAL;
};

#endif // CHARTVIEWSPECTRUMWB_H
