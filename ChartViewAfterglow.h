#ifndef CHARTVIEWAFTERGLOW_H
#define CHARTVIEWAFTERGLOW_H

#include "ChartViewCustom.h"

class ChartViewAfterglow: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewAfterglow(QString, double, double, double, double, QWidget* = nullptr);
    void replace(unsigned char* const buf);

protected:
    void rescaleKeyAxis(const QCPRange& range) override;

private:
    size_t DataSize = 0;
    int SeriesIndex = 0;
    static constexpr int SERIES_SIZE = 10;
    QCPGraph *SpectrumSeries[SERIES_SIZE];
};

#endif // CHARTVIEWAFTERGLOW_H
