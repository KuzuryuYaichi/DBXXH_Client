#include "ChartWidgetWB.h"

extern PARAMETER_SET g_parameter_set;

ChartWidgetWB::ChartWidgetWB(QString title, QWidget* parent): ChartWidgetCombine(title, parent)
{
    hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(new QLabel(tr("Domain:")), 1);
    hBoxLayout->addWidget(showBox = new QComboBox, 2);
    showBox->addItem(tr("Freq"), SPECTRUM_MODE);
    showBox->addItem(tr("Heapmap"), HEATMAP_MODE);
    showBox->addItem(tr("Afterflow"), AFTERFLOW_MODE);
    connect(showBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
        ChangeMode(showBox->currentData().toInt());
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Freq(MHz):")));
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

    layoutSpectrum->addWidget(chartSpectrum = new ChartViewSpectrumWB(title, MIN_FREQ, MAX_FREQ, MIN_AMPL, MAX_AMPL));
    layoutSpectrum->addWidget(chartHeatmap = new ChartViewHeatmap(title, MIN_FREQ, MAX_FREQ, MIN_AMPL, MAX_AMPL));
    layoutSpectrum->addWidget(chartAfterglow = new ChartViewAfterglow(title, MIN_FREQ, MAX_FREQ, MIN_AMPL, MAX_AMPL));

    connect(chartSpectrum->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](const QCPRange& newRange) {
        chartWaterfall->xAxis->setRange(newRange);
        chartHeatmap->xAxis->setRange(newRange);
        chartAfterglow->xAxis->setRange(newRange);
        chartWaterfall->replot();
    });
    connect(chartHeatmap->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](const QCPRange& newRange) {
        chartSpectrum->xAxis->setRange(newRange);
        chartWaterfall->xAxis->setRange(newRange);
        chartAfterglow->xAxis->setRange(newRange);
        chartWaterfall->replot();
    });
    connect(chartAfterglow->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](const QCPRange& newRange) {
        chartSpectrum->xAxis->setRange(newRange);
        chartHeatmap->xAxis->setRange(newRange);
        chartWaterfall->xAxis->setRange(newRange);
        chartWaterfall->replot();
    });
    connect(chartWaterfall->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](const QCPRange& newRange) {
        chartSpectrum->xAxis->setRange(newRange);
        chartHeatmap->xAxis->setRange(newRange);
        chartAfterglow->xAxis->setRange(newRange);
        if (chartSpectrum->isVisible())
            chartSpectrum->replot();
        if (chartHeatmap->isVisible())
            chartHeatmap->replot();
        if (chartAfterglow->isVisible())
            chartAfterglow->replot();
    });

    chartHeatmap->hide();
    chartAfterglow->hide();
    chartSpectrum->show();

    connect(freqEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (freqEdit->hasFocus())
        {
            g_parameter_set.CenterFreq = freqEdit->value() * 1e6;
            emit ParamsChanged();
        }
    });

    hBoxLayout->addWidget(new QLabel(tr("Sim Band(MHz):")));
    hBoxLayout->addWidget(boundBox = new QComboBox);
    boundBox->addItem("0.9375", 64);
    boundBox->addItem("1.875", 32);
    boundBox->addItem("3.75", 16);
    boundBox->addItem("7.5", 8);
    boundBox->addItem("15", 4);
    boundBox->addItem("30",2);
    boundBox->setCurrentIndex(boundBox->count() - 1);
    connect(boundBox, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if (index < 0)
            return;
        for (int i = 0; i < 5; ++i)
        {
            freqResBox->setItemText(i, QString::number(RESOLUTIONS[index + i]));
        }
        g_parameter_set.FreqRes = freqResBox->currentData().toInt();
        g_parameter_set.SimBW = boundBox->currentData().toInt();
        emit ParamsChanged();
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Resolution(kHz):")));
    hBoxLayout->addWidget(freqResBox = new QComboBox);
    auto index = boundBox->currentIndex();
    for (auto i = 0; i < 5; ++i)
    {
        freqResBox->addItem(QString::number(RESOLUTIONS[index + i]), 14 - i);
    }
    freqResBox->setCurrentIndex(0);
    connect(freqResBox, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if(index < 0)
            return;
        g_parameter_set.FreqRes = freqResBox->itemData(index).toInt();
        emit ParamsChanged();
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Smooth:")));
    hBoxLayout->addWidget(smoothBox = new QComboBox);
    for (auto i = 1; i <= 32; i *= 2)
    {
        smoothBox->addItem(QString::number(i), i);
    }
    connect(smoothBox, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if (index < 0)
            return;
        g_parameter_set.SmNum = smoothBox->itemData(index).toUInt();
        emit ParamsChanged();
    });
    hBoxLayout->addStretch(1);

    RfGainModeGroup = new QButtonGroup(this);
    QHBoxLayout* pLayout;
    hBoxLayout->addWidget(new QLabel(tr("Rf Gain Mode:")));
    hBoxLayout->addLayout(pLayout = new QHBoxLayout);
    for (int i = 0; i < 2; ++i)
    {
        static QString RadioText[] = { tr("MGC"), tr("AGC") };
        auto pButton = new QRadioButton(RadioText[i], this);
        pLayout->addWidget(pButton);
        RfGainModeGroup->addButton(pButton);
        RfGainModeGroup->setId(pButton, i);
    }
    RfGainModeGroup->button(0)->setChecked(true);
    connect(RfGainModeGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton*) {
        g_parameter_set.GainMode = RfGainModeGroup->checkedId();
        emit ParamsChanged();
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Rf Desc(0~31dB):")));
    hBoxLayout->addWidget(RfGainEdit = new QDoubleSpinBox);
    RfGainEdit->setMinimum(0);
    RfGainEdit->setMaximum(31);
    RfGainEdit->setSingleStep(1);
    RfGainEdit->setDecimals(0);
    RfGainEdit->setValue(0);
    connect(RfGainEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (RfGainEdit->hasFocus())
        {
            g_parameter_set.Rf_MGC = RfGainEdit->text().toUInt();
            emit ParamsChanged();
        }
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Digit Desc(0~30dB):")));
    hBoxLayout->addWidget(DigitGainEdit = new QDoubleSpinBox);
    DigitGainEdit->setMinimum(0);
    DigitGainEdit->setMaximum(30);
    DigitGainEdit->setSingleStep(1);
    DigitGainEdit->setDecimals(0);
    DigitGainEdit->setValue(0);
    connect(DigitGainEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (DigitGainEdit->hasFocus())
        {
            g_parameter_set.Digit_MGC = DigitGainEdit->text().toUInt();
            emit ParamsChanged();
        }
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Feedback State:")));
    FeedbackGroup = new QButtonGroup(this);
    hBoxLayout->addLayout(pLayout = new QHBoxLayout);
    for (int i = 0; i < 2; ++i)
    {
        static QString RadioText[] = { tr("Close"), tr("Open") };
        auto pButton = new QRadioButton(RadioText[i], this);
        pLayout->addWidget(pButton);
        FeedbackGroup->addButton(pButton);
        FeedbackGroup->setId(pButton, i);
    }
    FeedbackGroup->button(0)->setChecked(true);
    connect(FeedbackGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton*) {
        g_parameter_set.Feedback = FeedbackGroup->checkedId();
        emit ParamsChanged();
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Series State:")));
    hBoxLayout->addWidget(MaxKeepSelect = new QCheckBox(tr("Max Keep")));
    connect(MaxKeepSelect, &QCheckBox::stateChanged, this, &ChartWidgetWB::SeriesSelectChanged);
    hBoxLayout->addWidget(MinKeepSelect = new QCheckBox(tr("Min Keep")));
    connect(MinKeepSelect, &QCheckBox::stateChanged, this, &ChartWidgetWB::SeriesSelectChanged);
}

void ChartWidgetWB::SeriesSelectChanged()
{
    chartSpectrum->SeriesSelectChanged(MaxKeepSelect->checkState(), MinKeepSelect->checkState());
}

void ChartWidgetWB::ChangeMode(int index)
{
    switch (index)
    {
    case SPECTRUM_MODE:
    {
        chartHeatmap->hide();
        chartAfterglow->hide();
        chartSpectrum->show();
        break;
    }
    case HEATMAP_MODE:
    {
        chartSpectrum->hide();
        chartAfterglow->hide();
        chartHeatmap->show();
        break;
    }
    case AFTERFLOW_MODE:
    {
        chartSpectrum->hide();
        chartHeatmap->hide();
        chartAfterglow->show();
        break;
    }
    }
}

void ChartWidgetWB::replace(unsigned char* const buf)
{
    auto param = (ParamPowerWB*)(buf + sizeof(DataHead));
    switch (showBox->currentData().toInt())
    {
    case SPECTRUM_MODE:
    {
        chartSpectrum->replace(buf);

        break;
    }
    case HEATMAP_MODE:
    {
        chartHeatmap->replace(buf);
        break;
    }
    case AFTERFLOW_MODE:
    {
        chartAfterglow->replace(buf);
        break;
    }
    }
//    chartWaterfall->replace(buf, param->StartFreq, param->StopFreq, param->DataPoint);
}
