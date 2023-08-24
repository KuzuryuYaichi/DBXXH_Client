#include "CombineWidget.h"

CombineWidget::CombineWidget(QString title, QString X_title, int AXISX_MIN, int AXISX_MAX, QString Y_title, int AXISY_MIN, int AXISY_MAX, QWidget* parent): QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(chartWaterfall = new ChartViewWaterfall(title, X_title, AXISX_MIN, AXISX_MAX, Y_title, AXISY_MIN, AXISY_MAX), 10);
    mainLayout->addLayout(layoutSpectrum = new QHBoxLayout, 10);
    layoutSpectrum->addWidget(chartWave = new ChartViewWave(title, X_title, AXISX_MIN, AXISX_MAX, Y_title, SHRT_MIN, SHRT_MAX));
    layoutSpectrum->addWidget(chartSpectrum = new ChartViewSpectrum(title, X_title, AXISX_MIN, AXISX_MAX, Y_title, AXISY_MIN, AXISY_MAX));
    chartWave->hide();
}

void CombineWidget::replace(unsigned char* const buf)
{
    chartSpectrum->replace(buf);
    chartWaterfall->replace(buf);
}

void CombineWidget::ChangeMode(int index)
{
    if (index == 0)
    {
        chartSpectrum->hide();
        chartWave->show();
    }
    else
    {
        chartWave->hide();
        chartSpectrum->show();
    }
}
