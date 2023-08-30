#include "ChartWB.h"

extern PARAMETER_SET g_parameter_set;

ChartWB::ChartWB(QString title, QWidget* parent): CombineWidget(title, parent)
{
    chartWave->hide();
    chartHeatmap->hide();
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
    boundBox->addItem("0.9375", 1);
    boundBox->addItem("1.875", 2);
    boundBox->addItem("3.75", 3);
    boundBox->addItem("7.5", 4);
    boundBox->addItem("15", 5);
    boundBox->addItem("30", 6);
    boundBox->setCurrentIndex(5);
    connect(boundBox, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if (index < 0)
            return;
        for (int i = 0; i < 5; ++i)
        {
            freqResBox->setItemText(i, QString::number(RESOLUTIONS[index + i]) + "KHz");
        }
        g_parameter_set.FreqRes = freqResBox->currentData().toInt();
        g_parameter_set.SimBW = boundBox->currentData().toInt();
        emit ParamsChanged();
        //        auto bandWidth = simBWBox->itemData(index).toDouble();
        //        auto center = centerEdit->value(),
        //            min = center - bandWidth / 2,
        //            max = center + bandWidth / 2;
        //        dfChart->setAxisxMin(min);
        //        dfChart->setAxisxMax(max);
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
    for (auto i = 4; i <= 10; ++i)
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
    connect(RfGainModeGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton*) {
        g_parameter_set.GainMode = RfGainModeGroup->checkedId();
        emit ParamsChanged();
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Rf Desc(0~31dB):")));
    hBoxLayout->addWidget(RfGainEdit = new QDoubleSpinBox);
    RfGainEdit->setMinimum(0);
    RfGainEdit->setMaximum(60);
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

    hBoxLayout->addWidget(new QLabel(tr("Digit Desc(0~31dB):")));
    hBoxLayout->addWidget(DigitGainEdit = new QDoubleSpinBox);
    DigitGainEdit->setMinimum(0);
    DigitGainEdit->setMaximum(60);
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
    connect(FeedbackGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton*) {
        g_parameter_set.Feedback = FeedbackGroup->checkedId();
        emit ParamsChanged();
    });
}
