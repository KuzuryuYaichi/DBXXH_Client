#include "ChartViewHeatmap.h"

ChartViewHeatmap::ChartViewHeatmap(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Freq(MHz)"), tr("Ampl(dBm)"), parent)
{
    xAxis->setRange(xRange = { AXISX_MIN, AXISX_MAX });
    yAxis->setRange(yRange = { AXISY_MIN, AXISY_MAX });
    m_pColorMap = new QCPColorMap(xAxis, yAxis);
}

void ChartViewHeatmap::replace(unsigned char* const buf)
{
    int nx = 1024;
    int ny = 400;
    m_pColorMap->data()->setSize(nx, ny);

    double sx = 419.825,ex = 419.975;
    m_pColorMap->data()->setRange(QCPRange(sx, ex), QCPRange(-25, 100));

    for (auto xIndex = 0; xIndex < nx; ++xIndex)
    {
        for (auto yIndex = 0; yIndex < ny; ++yIndex)
        {
            auto y = 2 * ny / 5 + 40 * qSin(xIndex / 360.0 * 2 * M_PI);
            auto z = 90 - 2.0 * fabs(y - yIndex);
            m_pColorMap->data()->setCell(xIndex, yIndex, z);
        }
    }
    m_pColorMap->setGradient(QCPColorGradient::gpJet);
    rescaleAxes();
}
