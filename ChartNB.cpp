#include "ChartNB.h"

#include <QStyle>

ChartNB::ChartNB(QString title, QWidget* parent): CombineWidget(title, parent)
{
    chartSpectrum->hide();
    chartWave->show();
    auto layoutAdjust = new QWidget;
    mainLayout->insertWidget(0, layoutAdjust, 1);
    auto hBoxLayout = new QHBoxLayout(layoutAdjust);
    hBoxLayout->addWidget(new QLabel(tr("NB Freq(MHz):")), 1);
    hBoxLayout->addWidget(freqEdit = new QDoubleSpinBox, 2);
    freqEdit->setMinimum(MIN_FREQ);
    freqEdit->setMaximum(MAX_FREQ);
    freqEdit->setSingleStep(1);
    freqEdit->setDecimals(3);
    connect(freqEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (freqEdit->hasFocus())
        {
            emit FreqBandwidthChanged(freqEdit->value() * 1e6, bandBox->currentData().toULongLong(), demodBox->currentData().toUInt());
        }
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("BW(kHz):")), 1);
    hBoxLayout->addWidget(bandBox = new QComboBox, 2);
    bandBox->addItem("0.15", 150);
    bandBox->addItem("0.3", 300);
    bandBox->addItem("0.6", 600);
    bandBox->addItem("1.5", 1500);
    bandBox->addItem("2.4", 2400);
    bandBox->addItem("6", 6000);
    bandBox->addItem("9", 9000);
    bandBox->addItem("15", 15000);
    bandBox->addItem("30", 30000);
    bandBox->addItem("50", 50000);
    bandBox->addItem("120", 120000);
    bandBox->addItem("150", 150000);
    connect(bandBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
        emit FreqBandwidthChanged(freqEdit->value() * 1e6, bandBox->currentData().toULongLong(), demodBox->currentData().toUInt());
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Demod:")), 1);
    hBoxLayout->addWidget(demodBox = new QComboBox, 2);
    static constexpr const char* DEMOD_TYPE[] = { "IQ", "AM", "FM", "PM", "USB", "LSB", "ISB", "CW", "FSK" };
    for (auto i = 0ull; i < 8; ++i)
        demodBox->addItem(DEMOD_TYPE[i], i);
    connect(demodBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
        emit FreqBandwidthChanged(freqEdit->value() * 1e6, bandBox->currentData().toULongLong(), demodBox->currentData().toUInt());
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Domain:")), 1);
    hBoxLayout->addWidget(showBox = new QComboBox, 2);
    showBox->addItem(tr("Time"), DDC_MODE);
    showBox->addItem(tr("Freq"), FFT_MODE);
    connect(showBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int index) {
        ChangeMode(index);
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Play:")), 1);
    auto style = QApplication::style();
    hBoxLayout->addWidget(playBtn = new QPushButton(style->standardIcon(QStyle::SP_MediaPlay), ""), 2);
    connect(playBtn, &QPushButton::clicked, this, [this] {
        changedListening(playing);
        emit triggerListening(playing = !playing);
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Rec:")), 1);
    hBoxLayout->addWidget(playBtn = new QPushButton(style->standardIcon(QStyle::SP_DialogNoButton), ""), 2);
    connect(playBtn, &QPushButton::clicked, this, [this] {
        changedRecording(recording);
    });
    hBoxLayout->addStretch(1);
}

void ChartNB::changedListening(bool state)
{
    this->playing = state;
    auto style = QApplication::style();
    playBtn->setIcon(style->standardIcon(playing? QStyle::SP_MediaPlay: QStyle::SP_MediaPause));
}

void ChartNB::changedRecording(bool state)
{
    this->recording = state;
    auto style = QApplication::style();
    playBtn->setIcon(style->standardIcon(recording? QStyle::SP_DialogNoButton: QStyle::SP_MediaPause));
}

void ChartNB::replace(unsigned char* const buf)
{
    switch (showBox->currentData().toInt())
    {
    case DDC_MODE:
    {
        chartWave->replace(buf);
        break;
    }
    case FFT_MODE:
    {
        chartSpectrum->replace(buf);
        break;
    }
    }
}
