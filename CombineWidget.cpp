#include "CombineWidget.h"

CombineWidget::CombineWidget(QString title, QWidget* parent): QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(chartWaterfall = new ChartViewWaterfall(title, MIN_FREQ, MAX_FREQ, -WATERFALL_DEPTH, 0), 10);
    mainLayout->addLayout(layoutSpectrum = new QHBoxLayout, 10);
    layoutSpectrum->addWidget(chartWave = new ChartViewWave(title, 0, DDC_LEN, SHRT_MIN, SHRT_MAX));
    layoutSpectrum->addWidget(chartSpectrum = new ChartViewSpectrum(title, MIN_FREQ, MAX_FREQ, MIN_AMPL, MAX_AMPL));

    connect(chartSpectrum->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](const QCPRange& newRange) {
        chartWaterfall->xAxis->setRange(newRange);
        chartWaterfall->replot();
    });
    connect(chartWaterfall->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](const QCPRange& newRange) {
        chartSpectrum->xAxis->setRange(newRange);
        chartSpectrum->replot();
    });
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
