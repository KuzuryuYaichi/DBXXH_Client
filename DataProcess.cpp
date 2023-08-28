#include "DataProcess.h"

#include "global.h"
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
        m_wbWidget->sigSetValidAmpThreshold(-60);
        while (true)
        {
            auto [res, packets] = queue.wait_and_pop();
            if (!res)
                return;
            auto buf = packets.get();
            auto head = (DataHead*)buf;
            switch (head->PackType)
            {
            case 0x515:
            {
                auto param = (ParamPowerWB*)(buf + sizeof(DataHead));

                auto BAND_WIDTH = (param->StopFreq - param->StartFreq);
                auto CenterFreq = (param->StopFreq + param->StartFreq) / 2;
//                auto freq_step = ResolveResolution(param->Resolution, BAND_WIDTH);
                auto amplData = (unsigned char*)(buf + sizeof(DataHead) + sizeof(ParamPowerWB));
                m_wbWidget->sigTriggerSignalDetect(amplData, 32, param->DataPoint, CenterFreq, BAND_WIDTH);
                break;
            }
            default: return;
            }

        }
        qDebug() << "DataProcess Exited";
    });
}
