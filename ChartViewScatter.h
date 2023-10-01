#ifndef CHARTVIEWSCATTER_H
#define CHARTVIEWSCATTER_H

#include "ChartViewCustom.h"

class ChartViewScatter: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewScatter(QString, double, double, double, double, QWidget* = nullptr);
    void replace(unsigned char* const buf);

protected:
    QCPGraph *ScatterSeries;
};

#endif // CHARTVIEWSCATTER_H
