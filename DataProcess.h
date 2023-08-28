#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include "inc/WBSignalDetectWidget.h"

#include <tuple>
#include <vector>
#include "ThreadSafeQueue.h"

using DATA_TYPE = std::shared_ptr<unsigned char[]>;

class DataProcess
{
public:
    DataProcess(WBSignalDetectWidget*);
    ~DataProcess();
    void ProcessData();
    void execute(const DATA_TYPE& task);

private:
    WBSignalDetectWidget* m_wbWidget;
    std::pair<double, double> DirectionConfidence(int*);
    threadsafe_queue<DATA_TYPE> queue;
    std::thread ProcessThread;
};

#endif // DATAPROCESS_H
