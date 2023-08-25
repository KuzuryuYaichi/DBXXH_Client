#ifndef CHARTVIEWWATERFALL_H
#define CHARTVIEWWATERFALL_H

#include "ChartViewCustom.h"
#include "StructNetData.h"
#include <mutex>

class ChartViewWaterfall: public ChartViewCustom
{
    Q_OBJECT
public:
    ChartViewWaterfall(QString, double, double, double, double, QWidget* = nullptr);
    void replace(unsigned char* const);

private:
    void UpdateAnalyzeDataByCell();
    void RegenerateParams(long long, long long, size_t);
    void analyzeFrame(unsigned char*, size_t);
    void analyzeFrame(StructSweepRangeDirectionData*, size_t);
    QCPColorMap* m_pColorMap;
    std::list<std::vector<short>> points;
    std::vector<short> pointsAnalyze;
    bool readyData = true;

    static constexpr int REFRESH_INTERVAL = 1000;
    static constexpr int MIN_AMPL_WATERFALL = -120, MAX_AMPL_WATERFALL = 0;
};

#endif // CHARTVIEWWATERFALL_H
