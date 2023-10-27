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
    void triggerFM_Index(double);
    void UpdateThreshold(float);

protected:
    QCPGraph *MaxKeepSeries, *MinKeepSeries, *TrackSeries, *FM_IndexSeries;
    QCPItemTracer *TracerMarker[MARKER_NUM];
    QCPItemText* TracerText[MARKER_NUM];
    QCPLayoutGrid* MarkElement;
    double TrackStartFreq = 0, TrackEndFreq = 0, FM_StartFreq = 0, FM_EndFreq = 0;
    void AnalyzeFrame(long long);
    void AnalyzeMark(double, double, unsigned char*, int);
    void AnalyzeMeasure(double, double, int);
    void AnalyzeTrack(double, double, unsigned char*, int);
    void AnalyzeFM_Index(double, double, int);
    QVector<double> pointsMax, pointsMin;
    bool MaxKeepSelect = false, MinKeepSelect = false;
    QMenu* menu;
    bool MenuAppear = true;
    void UpdateTrack(QMouseEvent *);
    void UpdateIndex(QMouseEvent *);
    void UpdateRuler(QMouseEvent *);
    void InitMenu();
    enum DISPLAY {
        NORMAL,
        MARK,
        FM_INDEX,
        TRACK
    } DisplayState = NORMAL;
};

#endif // CHARTVIEWSPECTRUMWB_H
