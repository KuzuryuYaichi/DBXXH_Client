#ifndef CHARTVIEWWATERFALL_H
#define CHARTVIEWWATERFALL_H

#include "ChartViewCustom.h"
#include "StructNetData.h"

class ChartViewWaterfall: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewWaterfall(QString, QString, int, int, QString, int, int, QWidget* = nullptr);
    void replace(char* const);

private:
    void RegenerateParams(long long, long long, int);
    void addFrame(unsigned char*, int);
    void addFrame(StructSweepRangeDirectionData*, int);
    QCPColorMap* m_pColorMap;
    std::list<std::vector<double>> points;
    int xLength = 0;
};

#endif // CHARTVIEWWATERFALL_H
