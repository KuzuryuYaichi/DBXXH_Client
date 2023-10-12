#include "DataProcess.h"

#include "StructNetData.h"

DataProcess::DataProcess(WBSignalDetectWidget* m_wbWidget): m_wbWidget(m_wbWidget) {}

DataProcess::~DataProcess()
{
    queue.clean();
    if (ProcessThread.joinable())
        ProcessThread.join();
    qDebug() << "~DataProcess()";
}

void DataProcess::execute(const DATA_TYPE& task)
{
    queue.push(task);
}

void DataProcess::ProcessData()
{
    ProcessThread = std::thread([this]
    {
        while (true)
        {
            auto [res, packets] = queue.wait_and_pop();
            if (!res)
                return;
            auto buf = packets.get();
            auto param = (ParamPowerWB*)(buf + sizeof(DataHead));
            auto BAND_WIDTH = (param->StopFreq - param->StartFreq);
            auto amplData = (unsigned char*)(buf + sizeof(DataHead) + sizeof(ParamPowerWB));
            m_wbWidget->sigTriggerSignalDetect(amplData, param->DataPoint, param->StartFreq, BAND_WIDTH);
        }
    });
}
