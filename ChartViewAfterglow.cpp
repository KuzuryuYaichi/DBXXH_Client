#include "ChartViewAfterglow.h"

#include "StructNetData.h"

ChartViewAfterglow::ChartViewAfterglow(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Freq(MHz)"), tr("Ampl(dBm)"), parent)
{
    xAxis->setRange(xRange = { AXISX_MIN, AXISX_MAX });
    yAxis->setRange(yRange = { AXISY_MIN, AXISY_MAX });

    for (auto i = 0; i < SERIES_SIZE; ++i)
    {
        SpectrumSeries[i] = addGraph();
        SpectrumSeries[i]->setLineStyle(QCPGraph::lsLine);
        SpectrumSeries[i]->rescaleAxes(true);
    }
}

void ChartViewAfterglow::replace(unsigned char* const buf)
{
    if (!ready)
        return;
    ready = false;
    auto param = (ParamPowerWB*)(buf + sizeof(DataHead));
    auto BAND_WIDTH = (param->StopFreq - param->StartFreq) / 1e6;
    auto freq_step = ResolveResolution(param->Resolution, BAND_WIDTH);
    auto start_freq = param->StartFreq / 1e6;
    auto amplData = (unsigned char*)(buf + sizeof(DataHead) + sizeof(ParamPowerWB));
    QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
    auto x = start_freq;
    for (int i = 0; i < param->DataPoint; ++i)
    {
        amplx[i] = x;
        double y = (short)amplData[i] + AMPL_OFFSET;
        amply[i] = y;
        x += freq_step;
    }
    SpectrumSeries[SeriesIndex]->setData(amplx, amply);
    for (int i = 0; i < SERIES_SIZE; ++i)
    {
        SpectrumSeries[(SeriesIndex + SERIES_SIZE) % SERIES_SIZE]->setPen(QPen(QColor(0, 0, 255, 255 * (SERIES_SIZE - i) / SERIES_SIZE)));
    }
    QCPRange range(param->StartFreq / 1e6, param->StopFreq / 1e6);
    xRangeChanged(range);
    if (--SeriesIndex < 0)
        SeriesIndex = SERIES_SIZE - 1;
    replot(QCustomPlot::rpQueuedReplot);
}
