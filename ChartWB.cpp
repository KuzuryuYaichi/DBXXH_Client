#include "ChartWB.h"

ChartWB::ChartWB(QString title, QWidget* parent): CombineWidget(title, parent)
{
    chartWave->hide();
    chartSpectrum->show();
}
