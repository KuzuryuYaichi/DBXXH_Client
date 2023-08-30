#ifndef COMBINEWIDGET_H
#define COMBINEWIDGET_H

#include "ChartViewWave.h"
#include "ChartViewWaterfall.h"
#include "ChartViewSpectrum.h"
#include "ChartViewHeatmap.h"

class CombineWidget: public QWidget
{
    Q_OBJECT
public:
    CombineWidget(QString, QWidget* = nullptr);
    void replace(unsigned char* const);
    void ChangeMode(int);

protected:
    QDoubleSpinBox* freqEdit;
    QComboBox* boundBox;
    QComboBox* showBox;
    enum SHOW_MODE
    {
        WAVE_MODE = 0,
        SPECTRUM_MODE,
        HEATMAP_MODE,
        AFTERFLOW_MODE
    };
    QVBoxLayout* mainLayout;
    QHBoxLayout* layoutSpectrum;
    ChartViewWave* chartWave;
    ChartViewWaterfall* chartWaterfall;
    ChartViewSpectrum* chartSpectrum;
    ChartViewHeatmap* chartHeatmap;
    QHBoxLayout* hBoxLayout;
};

#endif // COMBINEWIDGET_H
