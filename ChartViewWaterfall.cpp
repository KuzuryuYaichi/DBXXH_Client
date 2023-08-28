#include "ChartViewWaterfall.h"

ChartViewWaterfall::ChartViewWaterfall(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Freq(MHz)"), tr("Time(Tick)"), parent, REFRESH_INTERVAL), points(WATERFALL_DEPTH)
{
    m_pColorMap = new QCPColorMap(xAxis, yAxis);
    m_pColorMap->data()->setValueSize(WATERFALL_DEPTH);
    m_pColorMap->data()->setRange(xRange = { AXISX_MIN, AXISX_MAX }, yRange = { AXISY_MIN, AXISY_MAX });
    m_pColorMap->setGradient(QCPColorGradient::gpHot);
    m_pColorMap->rescaleDataRange(true);
    rescaleAxes();

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event) {
        double key, value;
        m_pColorMap->pixelsToCoords(event->position(), key, value);
        auto data = m_pColorMap->data()->data(key, value);
        QToolTip::showText(event->globalPosition().toPoint(), QString("%1MHz, %2dBm").arg(key, 0,  'f', DECIMALS_PRECISION).arg(data));
        replot();
    });
}

void ChartViewWaterfall::RegenerateParams(long long StartFreq, long long StopFreq, size_t DataPoint)
{
    m_pColorMap->data()->setKeyRange(xRange = {StartFreq / 1e6, StopFreq / 1e6});
    if (points.front().size() != DataPoint)
    {
        for (auto iter = points.begin(); iter != points.end(); ++iter)
        {
            *iter = std::vector<short>(DataPoint, MIN_AMPL_WATERFALL);
        }
    }
    if (pointsAnalyze.size() != DataPoint)
    {
        pointsAnalyze.assign(DataPoint, MIN_AMPL_WATERFALL);
    }
}

void ChartViewWaterfall::analyzeFrame(unsigned char* amplData, size_t DataPoint)
{
//    if (pointsAnalyze.size() != DataPoint)
    {
        pointsAnalyze.assign(DataPoint, MIN_AMPL_WATERFALL);
    }
    for (auto i = 0ull; i < DataPoint; ++i)
    {
        pointsAnalyze[i] = std::max((short)((short)amplData[i] + AMPL_OFFSET), pointsAnalyze[i]);
    }
}

void ChartViewWaterfall::analyzeFrame(StructSweepRangeDirectionData* dataRangeDirection, size_t DataPoint)
{
//    if (pointsAnalyze.size() != DataPoint)
    {
        pointsAnalyze.assign(DataPoint, MIN_AMPL_WATERFALL);
    }
    for (auto i = 0ull; i < DataPoint; ++i)
    {
        pointsAnalyze[i] = std::max((short)((short)dataRangeDirection[i].Range + AMPL_OFFSET), pointsAnalyze[i]);
    }
}

void ChartViewWaterfall::UpdateAnalyzeDataByCell()
{
    if (!ready)
        return;
    ready = false;
    auto DataPoint = points.front().size();
    m_pColorMap->data()->setKeySize(DataPoint);
    int y = 0;
    for (auto iter = points.begin(); iter != points.end(); ++iter, ++y)
    {
        for (size_t x = 0; x < DataPoint; ++x)
        {
            m_pColorMap->data()->setCell(x, y, (*iter)[x]);
        }
    }
    m_pColorMap->rescaleDataRange(true);
    replot(QCustomPlot::rpQueuedReplot);
}

void ChartViewWaterfall::replace(unsigned char* const buf)
{
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x515:
    {
        auto param = (ParamPowerWB*)(buf + sizeof(DataHead));
        auto DataPoint = param->DataPoint;
        RegenerateParams(param->StartFreq, param->StopFreq, DataPoint);
        const auto GROUP_LENGTH = sizeof(long long) + (sizeof(char) + sizeof(short)) * DataPoint;
        auto data = buf + sizeof(DataHead) + sizeof(ParamPowerWB);
//        for (int g = 0; g < param->CXGroupNum; ++g)
        {
            auto amplData = buf + sizeof(DataHead) + sizeof(ParamPowerWB);
            analyzeFrame(amplData, DataPoint);
            data += GROUP_LENGTH;
        }
        break;
    }
    default: return;
    }
    if (readyData)
    {
        readyData = false;
        points.pop_front();
        points.emplace_back(std::move(pointsAnalyze));
        std::thread([this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(REFRESH_INTERVAL));
            readyData = true;
        }).detach();
    }
    UpdateAnalyzeDataByCell();
}
