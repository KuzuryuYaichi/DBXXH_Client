#ifndef COMBINEWIDGET_H
#define COMBINEWIDGET_H

#include "ChartViewSpectrum.h"
#include "ChartViewWaterfall.h"

class CombineWidget: public QWidget
{
    Q_OBJECT
public:
    CombineWidget(QString = "", QString = "", int = 0, int = 0, QString = "", int = 0, int = 0, QWidget* = nullptr);
    void replace(unsigned char* const);

protected:
    QVBoxLayout* mainLayout;
    ChartViewSpectrum* chartSpectrum;
    ChartViewWaterfall* chartWaterfall;
};

#endif // COMBINEWIDGET_H
