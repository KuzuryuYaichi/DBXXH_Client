#ifndef CHARTVIEWHEATMAP_H
#define CHARTVIEWHEATMAP_H

#include "ChartViewCustom.h"

class ChartViewHeatmap: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewHeatmap(QString, double, double, double, double, QWidget* = nullptr);
    void replace(unsigned char* const buf);

private:
    QCPColorMap* m_pColorMap;
};

#endif // CHARTVIEWHEATMAP_H
