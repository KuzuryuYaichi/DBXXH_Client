#include "CombineWidget.h"

CombineWidget::CombineWidget(QString title, QWidget* parent): QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);

    hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(new QLabel(tr("Domain:")), 1);
    hBoxLayout->addWidget(showBox = new QComboBox, 2);
    showBox->addItem(tr("Time"), WAVE_MODE);
    showBox->addItem(tr("Freq"), SPECTRUM_MODE);
    showBox->addItem(tr("Heapmap"), HEATMAP_MODE);
    showBox->addItem(tr("Afterflow"), AFTERFLOW_MODE);
    connect(showBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int index) {
        ChangeMode(index);
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Center Freq(MHz):")));
    hBoxLayout->addWidget(freqEdit = new QDoubleSpinBox);
    freqEdit->setMinimum(MIN_FREQ);
    freqEdit->setMaximum(MAX_FREQ);
    freqEdit->setSingleStep(1);
    freqEdit->setDecimals(6);
    freqEdit->setValue(MID_FREQ);
    hBoxLayout->addStretch(1);

    mainLayout->addLayout(hBoxLayout, 1);
    mainLayout->addWidget(chartWaterfall = new ChartViewWaterfall(title, MIN_FREQ, MAX_FREQ, -WATERFALL_DEPTH, 0), 10);
    mainLayout->addLayout(layoutSpectrum = new QHBoxLayout, 10);
    layoutSpectrum->addWidget(chartWave = new ChartViewWave(title, 0, DDC_LEN, SHRT_MIN, SHRT_MAX));
    layoutSpectrum->addWidget(chartSpectrum = new ChartViewSpectrum(title, MIN_FREQ, MAX_FREQ, MIN_AMPL, MAX_AMPL));
    layoutSpectrum->addWidget(chartHeatmap = new ChartViewHeatmap(title, MIN_FREQ, MAX_FREQ, MIN_AMPL, MAX_AMPL));

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
