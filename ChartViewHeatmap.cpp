#include "ChartViewHeatmap.h"

ChartViewHeatmap::ChartViewHeatmap(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Freq(MHz)"), tr("Ampl(dBm)"), parent)
{
    xAxis->setRange(xRange = { AXISX_MIN, AXISX_MAX });
    yAxis->setRange(yRange = { AXISY_MIN, AXISY_MAX });
    m_pColorMap = new QCPColorMap(xAxis, yAxis);
    m_pColorMap->setGradient(QCPColorGradient::gpJet);
    m_pColorMap->data()->setValueSize(AMPL_POINTS);
    m_pColorMap->data()->setValueRange(QCPRange(MIN_AMPL, MAX_AMPL));

    auto colorScale = new QCPColorScale(this);
    colorScale->setDataRange(QCPRange(0,100));
    m_pColorMap->setColorScale(colorScale);
}

void ChartViewHeatmap::rescaleKeyAxis(const QCPRange& range)
{
    if (range != xRange)
    {
        rescaleKeyAxis(xRange);
    }
}

void ChartViewHeatmap::replace(unsigned char* const buf)
{
    if (!ready)
        return;
    ready = false;

    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x515:
    {
        auto param = (ParamPowerWB*)(buf + sizeof(DataHead));
        auto amplData = (unsigned char*)(buf + sizeof(DataHead) + sizeof(ParamPowerWB));
        m_pColorMap->data()->setKeySize(param->DataPoint);
        m_pColorMap->data()->setKeyRange(QCPRange(param->StartFreq / 1e6, param->StopFreq / 1e6));
        for (auto xIndex = 0; xIndex < param->DataPoint; ++xIndex)
        {
            for (auto yIndex = 0; yIndex < AMPL_POINTS; ++yIndex)
            {
                auto z = 90 - 2.0 * std::fabs(yIndex - amplData[xIndex]);
                m_pColorMap->data()->setCell(xIndex, yIndex, z);
            }
        }
        QCPRange range(param->StartFreq / 1e6, param->StopFreq / 1e6);
        xRangeChanged(range);
        rescaleKeyAxis(range);
        break;
    }
    case 0x602:
    {
//        auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
//        auto data = (NarrowDDC*)(param + 1);
//        static const auto WINDOW = HanningWindow<DDC_LEN>();
//        for (auto p = 0; p < param->DataPoint; ++p)
//        {
//            inR[p][0] = data[p].I * WINDOW[p];
//            inR[p][1] = data[p].Q * WINDOW[p];
//        }
//        fftw_execute(planR);
//        const auto HALF_LEN = param->DataPoint / 2;
//        for (int p = 0; p < HALF_LEN; ++p)
//        {
//            std::swap(outR[p], outR[HALF_LEN + p]);
//        }
//        auto RealHalfBandWidth = NB_HALF_BANDWIDTH[11];
//        switch (param->BandWidth)
//        {
//        case 150: RealHalfBandWidth = NB_HALF_BANDWIDTH[0]; break;
//        case 300: RealHalfBandWidth = NB_HALF_BANDWIDTH[1]; break;
//        case 600: RealHalfBandWidth = NB_HALF_BANDWIDTH[2]; break;
//        case 1500: RealHalfBandWidth = NB_HALF_BANDWIDTH[3]; break;
//        case 2400: RealHalfBandWidth = NB_HALF_BANDWIDTH[4]; break;
//        case 6000: RealHalfBandWidth = NB_HALF_BANDWIDTH[5]; break;
//        case 9000: RealHalfBandWidth = NB_HALF_BANDWIDTH[6]; break;
//        case 15000: RealHalfBandWidth = NB_HALF_BANDWIDTH[7]; break;
//        case 30000: RealHalfBandWidth = NB_HALF_BANDWIDTH[8]; break;
//        case 50000: RealHalfBandWidth = NB_HALF_BANDWIDTH[9]; break;
//        case 120000: RealHalfBandWidth = NB_HALF_BANDWIDTH[10]; break;
//        case 150000: RealHalfBandWidth = NB_HALF_BANDWIDTH[11]; break;
//        }
//        double freq = param->Frequency / 1e6 - RealHalfBandWidth, step = RealHalfBandWidth * 2 / param->DataPoint;
//        QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
//        for (int p = 0; p < param->DataPoint; ++p)
//        {
//            amply[p] = 20 * std::log10(std::sqrt(std::pow(outR[p][0], 2.0) + std::pow(outR[p][1], 2.0))) + AMPL_OFFSET;
//            amplx[p] = freq;
//            freq += step;
//        }
        break;
    }
    default: return;
    }
    replot(QCustomPlot::rpQueuedReplot);
}
