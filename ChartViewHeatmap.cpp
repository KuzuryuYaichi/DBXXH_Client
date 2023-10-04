#include "ChartViewHeatmap.h"

#include "StructNetData.h"

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
void ChartViewHeatmap::replace(unsigned char* const buf)
{
    if (!ready)
        return;
    ready = false;
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
    replot(QCustomPlot::rpQueuedReplot);
}
