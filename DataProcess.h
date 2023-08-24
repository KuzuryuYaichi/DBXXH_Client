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

signals:
    void UpdateFreqListTable(const QVector<std::tuple<double, double, int>>&, const QDateTime&);

private:
    ChartWidget* m_cxWidget = nullptr;
    SqlData sqlData;
};

#endif // DATAPROCESS_H
