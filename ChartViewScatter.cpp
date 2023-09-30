#include "ChartViewScatter.h"

ChartViewScatter::ChartViewScatter(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Freq(MHz)"), tr("Ampl(dBm)"), parent)
{

}

void ChartViewScatter::replace(unsigned char* const buf)
{

}
