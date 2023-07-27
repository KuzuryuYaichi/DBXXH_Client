#include "ChartViewWaterfall.h"

ChartViewWaterfall::ChartViewWaterfall(QString title, QString X_title, int AXISX_MIN, int AXISX_MAX, QString Y_title, int AXISY_MIN, int AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, X_title, Y_title, parent),  points(WATERFALL_DEPTH)
{
    m_pColorMap = new QCPColorMap(xAxis, yAxis);
    m_pColorMap->data()->setValueSize(WATERFALL_DEPTH);
    m_pColorMap->data()->setRange(QCPRange(xMin = AXISX_MIN, xMax = AXISX_MAX), QCPRange(yMin = AXISY_MIN, yMax = AXISY_MAX));
    m_pColorMap->setGradient(QCPColorGradient::gpJet);
    m_pColorMap->rescaleDataRange(true);
    rescaleAxes();
}

void ChartViewWaterfall::RegenerateParams(long long StartFreq, long long StopFreq, int DataPoint)
{
    if (xLength != DataPoint)
    {
        for (auto iter = points.begin(); iter != points.end(); ++iter)
        {
            *iter = std::vector<double>(DataPoint);
        }
        m_pColorMap->data()->setKeySize(DataPoint);
        m_pColorMap->data()->setKeyRange(QCPRange(xMin = StartFreq / 1e3, xMax = StopFreq / 1e3));
        xLength = DataPoint;
    }
}

void ChartViewWaterfall::addFrame(unsigned char* amplData, int DataPoint)
{
    auto vec = std::move(points.front());
    points.pop_front();
    for (int i = 0; i < DataPoint; ++i)
    {
        vec[i] = (short)amplData[i] + AMPL_OFFSET;
    }
    points.emplace_back(std::move(vec));
}

void ChartViewWaterfall::addFrame(StructSweepRangeDirectionData* dataRangeDirection, int DataPoint)
{
    auto vec = std::move(points.front());
    points.pop_front();
    for (int i = 0; i < DataPoint; ++i)
    {
        vec[i] = (short)dataRangeDirection[i].Range + AMPL_OFFSET;
    }
    points.emplace_back(std::move(vec));
}

void ChartViewWaterfall::replace(char* const buf)
{
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x515:
    {
        auto param = (StructFixedCXResult*)(buf + sizeof(DataHead));
        auto DataPoint = param->DataPoint;
        RegenerateParams(param->CenterFreq - HALF_BAND_WIDTH_KHZ, param->CenterFreq + HALF_BAND_WIDTH_KHZ, DataPoint);
        const auto GROUP_LENGTH = sizeof(long long) + (sizeof(char) + sizeof(short)) * DataPoint;
        auto data = buf + sizeof(DataHead) + sizeof(StructFixedCXResult);
        for (int g = 0; g < param->CXGroupNum; ++g)
        {
            auto amplData = (unsigned char*)(data + sizeof(long long));
            addFrame(amplData, DataPoint);
            data += GROUP_LENGTH;
        }
        break;
    }
    case 0x513:
    {
        auto param = (StructSweepCXResult*)(buf + sizeof(DataHead));
        auto DataPoint = param->CXResultPoint;
        RegenerateParams(param->StartFreq, param->StopFreq, DataPoint);
        auto timeStruct = (StructSweepTimeData*)(buf + sizeof(DataHead) + sizeof(StructSweepCXResult));
        auto dataRangeDirection = (StructSweepRangeDirectionData*)(timeStruct + param->TimeNum);
        addFrame(dataRangeDirection, DataPoint);
        break;
    }
    default: return;
    }

    if (!ready)
        return;
    ready = false;
    int y = 0;
    for (auto iter = points.begin(); iter != points.end(); ++iter, ++y)
    {
        for (auto x = 0; x < xLength; ++x)
        {
            m_pColorMap->data()->setCell(x, y, (*iter)[x]);
        }
    }
    m_pColorMap->rescaleDataRange(true);
    rescaleAxes();
    replot(QCustomPlot::rpQueuedReplot);
}
