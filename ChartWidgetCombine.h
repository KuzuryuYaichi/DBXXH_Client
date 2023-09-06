#ifndef CHARTWIDGETCOMBINE_H
#define CHARTWIDGETCOMBINE_H

#include "ChartViewWave.h"
#include "ChartViewWaterfall.h"
#include "ChartViewSpectrum.h"
#include "ChartViewHeatmap.h"
#include "ChartViewAfterglow.h"

class ChartWidgetCombine: public QWidget
{
    Q_OBJECT
public:
    ChartWidgetCombine(QString, QWidget* = nullptr);
    void replace(unsigned char* const);
    void ChangeMode(int);

protected:
    QComboBox* showBox;
    QDoubleSpinBox* freqEdit;
    QComboBox* boundBox;
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
    ChartViewAfterglow* chartAfterglow;
    QHBoxLayout* hBoxLayout;
};

#endif // CHARTWIDGETCOMBINE_H
