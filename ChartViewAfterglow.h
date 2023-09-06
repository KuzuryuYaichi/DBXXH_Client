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
};

#endif // CHARTVIEWAFTERGLOW_H
