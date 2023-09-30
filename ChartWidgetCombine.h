#ifndef CHARTWIDGETCOMBINE_H
#define CHARTWIDGETCOMBINE_H

#include "ChartViewWaterfall.h"

class ChartWidgetCombine: public QWidget
{
    Q_OBJECT
public:
    ChartWidgetCombine(QString, QWidget* = nullptr);
    virtual void replace(unsigned char* const) = 0;
    virtual void ChangeMode(int) = 0;
    ChartViewWaterfall* chartWaterfall;

protected:
    QComboBox* showBox;
    QDoubleSpinBox* freqEdit;
    QComboBox* boundBox;
    QVBoxLayout* mainLayout;
    QHBoxLayout* layoutSpectrum;
    QHBoxLayout* hBoxLayout;
};

#endif // CHARTWIDGETCOMBINE_H
