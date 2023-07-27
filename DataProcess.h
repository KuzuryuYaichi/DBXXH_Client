#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include "ChartWidget.h"

#include <QObject>
#include <tuple>
#include <vector>
#include "global.h"
#include "SqlData.h"

class DataProcess: public QObject
{
    Q_OBJECT
public:
    DataProcess(QString, ChartWidget* = nullptr);
    ~DataProcess();
    void ProcessData(std::vector<std::shared_ptr<char[]>>);
    void PushData(int, short);
    void PopData(long long, double, QDateTime);

signals:
    void UpdateFreqListTable(const QVector<std::tuple<double, double, int>>&, const QDateTime&);

private:
    ChartWidget* m_cxWidget = nullptr;
    void ClearDirections(int);
    void ClearPointsAnalyze();

    static constexpr int MAX_FREQS = 6401;
    static constexpr int MAX_DIRECTIONS = UPPER_DIRECTION - LOWER_DIRECTION + 1;
    int PointsAnalyze[MAX_FREQS];
    int FreqArray[MAX_FREQS][MAX_DIRECTIONS];
    short Direction[MAX_FREQS];
    double Confidence[MAX_FREQS];
    SqlData sqlData;
};

#endif // DATAPROCESS_H
