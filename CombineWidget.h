#ifndef COMBINEWIDGET_H
#define COMBINEWIDGET_H

#include "ChartViewSpectrum.h"
#include "ChartViewWaterfall.h"
#include "ChartViewWave.h"

class CombineWidget: public QWidget
{
    Q_OBJECT
public:
    CombineWidget(QString, QWidget* = nullptr);
    void replace(unsigned char* const);
    void ChangeMode(int);

protected:
    QVBoxLayout* mainLayout;
    QHBoxLayout* layoutSpectrum;
    ChartViewSpectrum* chartSpectrum;
    ChartViewWaterfall* chartWaterfall;
    ChartViewWave* chartWave;
};

#endif // COMBINEWIDGET_H
