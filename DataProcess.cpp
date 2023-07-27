#include "DataProcess.h"

#include "global.h"
#include "Model.h"
#include "StructNetData.h"

DataProcess::DataProcess(QString connectionName, ChartWidget* m_cxWidget): m_cxWidget(m_cxWidget), sqlData(connectionName)
{
    for (int i = 0; i < MAX_FREQS; ++i)
        ClearDirections(i);
}

DataProcess::~DataProcess()
{

}

void DataProcess::ClearPointsAnalyze()
{
    static constexpr auto MEM_LENGTH = sizeof(int) * MAX_FREQS;
    std::memset(PointsAnalyze, 0, MEM_LENGTH);
}

void DataProcess::ClearDirections(int index)
{
    static constexpr auto MEM_LENGTH = sizeof(int) * MAX_DIRECTIONS;
    std::memset(FreqArray[index], 0, MEM_LENGTH);
}

void DataProcess::ProcessData(std::vector<std::shared_ptr<char[]>> packets)
{
    ClearPointsAnalyze();
    if (packets[0] == nullptr)
        return;
    auto buf = packets[0].get();
    auto param = (StructFixedCXResult*)(buf + sizeof(DataHead));
    auto CenterFreq = param->CenterFreq;
    auto Resolution = param->FreqResolution;
    auto timeData = buf + sizeof(DataHead) + sizeof(StructFixedCXResult);
    for (auto iter = packets.begin(); iter != packets.end(); ++iter)
    {
        if (*iter == nullptr)
            return;
        auto buf = iter->get();
        auto param = (StructFixedCXResult*)(buf + sizeof(DataHead));
        const auto GROUP_LENGTH = sizeof(long long) + (sizeof(char) + sizeof(short)) * param->DataPoint;
        auto data = buf + sizeof(DataHead) + sizeof(StructFixedCXResult);
        if (Resolution != param->FreqResolution || CenterFreq != param->CenterFreq)
            return;
        for (int g = 0; g < param->CXGroupNum; ++g)
        {
            auto amplData = (unsigned char*)(data + sizeof(long long));
            auto doaData = (short*)(amplData + param->DataPoint);
            for (int i = 0; i < param->DataPoint; ++i)
            {
                auto range = (short)amplData[i] + AMPL_OFFSET;
                if (range > m_cxWidget->threshold)
                {
                    PushData(i, doaData[i]);
                }
            }
            data += GROUP_LENGTH;
        }
    }
    PopData(CenterFreq - HALF_BAND_WIDTH_KHZ, Resolution, Model::timeConvert(*(unsigned long long*)timeData));
}

void DataProcess::PushData(int freqIndex, short doa)
{
    if (doa > UPPER_DIRECTION || doa < LOWER_DIRECTION)
        return;
    auto lower = std::max((short)(doa - PHASE_MISTAKE), LOWER_DIRECTION) - LOWER_DIRECTION;
    auto upper = std::min((short)(doa + PHASE_MISTAKE), UPPER_DIRECTION) - LOWER_DIRECTION;
    if (PointsAnalyze[freqIndex] == 0)
    {
        ClearDirections(freqIndex);
    }
    for (int doaIndex = lower; doaIndex <= upper; ++doaIndex)
    {
        ++FreqArray[freqIndex][doaIndex];
    }
    ++PointsAnalyze[freqIndex];
}

void DataProcess::PopData(long long StartFreq, double Resolution, QDateTime timeFormat)
{
    QVector<std::tuple<double, double, int>> doa;
    for (int i = 0; i < MAX_FREQS; ++i)
    {
        auto totalPoints = PointsAnalyze[i];
        if (totalPoints == 0)
            continue;
        auto doaData = FreqArray[i];
        auto res = std::max_element(doaData, doaData + MAX_DIRECTIONS);
        Direction[i] = res - doaData + LOWER_DIRECTION;
        Confidence[i] = *res * 100.0 / totalPoints;
        doa.append(std::tuple<double, double, int>((StartFreq + i * Resolution) / 1e3, Direction[i] / 10.0, Confidence[i]));
    }
    sqlData.Insert(doa, timeFormat);
    emit UpdateFreqListTable(doa, timeFormat);
}
