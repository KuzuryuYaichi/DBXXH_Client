#include "ChartViewAfterglow.h"

ChartViewAfterglow::ChartViewAfterglow(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Freq(MHz)"), tr("Ampl(dBm)"), parent)
{
    xAxis->setRange(xRange = { AXISX_MIN, AXISX_MAX });
    yAxis->setRange(yRange = { AXISY_MIN, AXISY_MAX });
}

void ChartViewAfterglow::rescaleKeyAxis(const QCPRange& range)
{
    if (range != xRange)
    {
        rescaleKeyAxis(xRange);
    }
}

void ChartViewAfterglow::replace(unsigned char* const buf)
{

}
