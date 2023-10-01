#include "ChartViewWaterfall.h"

#include "StructNetData.h"

ChartViewWaterfall::ChartViewWaterfall(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Freq(MHz)"), tr("Time(Tick)"), parent, REFRESH_INTERVAL), points(WATERFALL_DEPTH)
{
    m_pColorMap = new QCPColorMap(xAxis, yAxis);
    m_pColorMap->data()->setValueSize(WATERFALL_DEPTH);
    m_pColorMap->data()->setRange(xRange = { AXISX_MIN, AXISX_MAX }, yRange = { AXISY_MIN, AXISY_MAX });
    m_pColorMap->setGradient(QCPColorGradient::gpJet);
    m_pColorMap->rescaleDataRange(true);

//    auto colorScale = new QCPColorScale(this);
//    colorScale->setDataRange(QCPRange(MIN_AMPL, MAX_AMPL));
//    m_pColorMap->setColorScale(colorScale);

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
            *iter = std::vector<short>(DataPoint, MIN_AMPL);
        }
    }
    if (pointsAnalyze.size() != DataPoint)
    {
        pointsAnalyze.assign(DataPoint, MIN_AMPL);
    }
}

void ChartViewWaterfall::analyzeFrame(unsigned char* amplData, size_t DataPoint)
{
//    if (pointsAnalyze.size() != DataPoint)
    {
        pointsAnalyze.assign(DataPoint, MIN_AMPL);
    }
    for (auto i = 0ull; i < DataPoint; ++i)
    {
        pointsAnalyze[i] = std::max((short)((short)amplData[i] + AMPL_OFFSET), pointsAnalyze[i]);
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

void ChartViewWaterfall::replace(unsigned char* buf)
{
    auto param = (ParamPowerWB*)(buf + sizeof(DataHead));
    RegenerateParams(param->StartFreq, param->StopFreq, param->DataPoint);
    auto amplData = buf + sizeof(DataHead) + sizeof(ParamPowerWB);
    analyzeFrame(amplData, param->DataPoint);
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

void ChartViewWaterfall::replace(unsigned char* buf, unsigned char* fft_data)
{
    auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
    double HalfSpsBound = NB_HALF_BOUND_Hz[11];
    switch (param->Bound)
    {
    case 150: HalfSpsBound = NB_HALF_BOUND_Hz[0]; break;
    case 300: HalfSpsBound = NB_HALF_BOUND_Hz[1]; break;
    case 600: HalfSpsBound = NB_HALF_BOUND_Hz[2]; break;
    case 1500: HalfSpsBound = NB_HALF_BOUND_Hz[3]; break;
    case 2400: HalfSpsBound = NB_HALF_BOUND_Hz[4]; break;
    case 6000: HalfSpsBound = NB_HALF_BOUND_Hz[5]; break;
    case 9000: HalfSpsBound = NB_HALF_BOUND_Hz[6]; break;
    case 15000: HalfSpsBound = NB_HALF_BOUND_Hz[7]; break;
    case 30000: HalfSpsBound = NB_HALF_BOUND_Hz[8]; break;
    case 50000: HalfSpsBound = NB_HALF_BOUND_Hz[9]; break;
    case 120000: HalfSpsBound = NB_HALF_BOUND_Hz[10]; break;
    case 150000: HalfSpsBound = NB_HALF_BOUND_Hz[11]; break;
    }
    RegenerateParams(param->Frequency - HalfSpsBound, param->Frequency + HalfSpsBound, param->DataPoint);
    analyzeFrame(fft_data, param->DataPoint);
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
