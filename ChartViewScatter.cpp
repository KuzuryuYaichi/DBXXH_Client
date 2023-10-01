#include "ChartViewScatter.h"

#include "StructNetData.h"

ChartViewScatter::ChartViewScatter(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Freq(MHz)"), tr("Ampl(dBm)"), parent)
{
    xAxis->setRange(xRange = { AXISX_MIN, AXISX_MAX });
    yAxis->setRange(yRange = { AXISY_MIN, AXISY_MAX });

    ScatterSeries = addGraph();
    ScatterSeries->setName(tr("Scatter"));
    ScatterSeries->setPen(QPen(Qt::blue));
    ScatterSeries->setScatterStyle(QCPScatterStyle::ssDisc);
    ScatterSeries->setLineStyle(QCPGraph::lsNone);
    ScatterSeries->rescaleAxes(true);
}

void ChartViewScatter::replace(unsigned char* const buf)
{
    if (!ready)
        return;
    ready = false;
    auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
    auto amplData = (NarrowDDC*)(param + 1);
//    switch (demodBox)
//    {
//    case FSK:
//    {
//        auto ptr = std::make_unique<short[]>(param->DataPoint);
//        for (auto i = 0; i < param->DataPoint; ++i)
//        {
//            ptr[i] = amplData[i].I > 0? 1: 0;
//        }
//        WriteFile((char*)ptr.get(), sizeof(short) * param->DataPoint);
//        break;
//    }
//    case PSK:
//    {
        QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
        for (auto i = 0; i < param->DataPoint; ++i)
        {
            amplx[i] = amplData[i].I > 0? 1: 0;
            amply[i] = amplData[i].Q > 0? 1: 0;
        }
//        break;
//    }
//    default: return;
//    }
    ScatterSeries->setData(amplx, amply);
    replot(QCustomPlot::rpQueuedReplot);
}
