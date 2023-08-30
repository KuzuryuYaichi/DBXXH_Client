#include "ChartWidgetNB.h"

#include <QStyle>

ChartWidgetNB::ChartWidgetNB(QString title, QWidget* parent): ChartWidgetCombine(title, parent)
{
    chartSpectrum->hide();
    chartHeatmap->hide();
    chartWave->show();

    connect(freqEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (freqEdit->hasFocus())
        {
            emit ParamsChanged(freqEdit->value() * 1e6, boundBox->currentData().toULongLong(), demodBox->currentData().toUInt());
        }
    });

    hBoxLayout->addWidget(new QLabel(tr("BW(kHz):")), 1);
    hBoxLayout->addWidget(boundBox = new QComboBox, 2);
    boundBox->addItem("0.15", 150);
    boundBox->addItem("0.3", 300);
    boundBox->addItem("0.6", 600);
    boundBox->addItem("1.5", 1500);
    boundBox->addItem("2.4", 2400);
    boundBox->addItem("6", 6000);
    boundBox->addItem("9", 9000);
    boundBox->addItem("15", 15000);
    boundBox->addItem("30", 30000);
    boundBox->addItem("50", 50000);
    boundBox->addItem("120", 120000);
    boundBox->addItem("150", 150000);
    connect(boundBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
        emit ParamsChanged(freqEdit->value() * 1e6, boundBox->currentData().toULongLong(), demodBox->currentData().toUInt());
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Demod:")), 1);
    hBoxLayout->addWidget(demodBox = new QComboBox, 2);
    static constexpr const char* DEMOD_TYPE[] = { "IQ", "AM", "FM", "PM", "USB", "LSB", "ISB", "CW", "FSK" };
    for (auto i = 0ull; i < 8; ++i)
        demodBox->addItem(DEMOD_TYPE[i], i);
    connect(demodBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
        emit ParamsChanged(freqEdit->value() * 1e6, boundBox->currentData().toULongLong(), demodBox->currentData().toUInt());
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
    hBoxLayout->addWidget(recordBtn = new QPushButton(style->standardIcon(QStyle::SP_DialogNoButton), ""), 2);
    connect(recordBtn, &QPushButton::clicked, this, [this] {
        changedRecording();
    });
    hBoxLayout->addStretch(1);
}

void ChartWidgetNB::changedListening(bool state)
{
    playing = state;
    auto style = QApplication::style();
    playBtn->setIcon(style->standardIcon(playing? QStyle::SP_MediaPlay: QStyle::SP_MediaPause));
}

void ChartWidgetNB::changedRecording()
{
    recording = !recording;
    auto style = QApplication::style();
    recordBtn->setIcon(style->standardIcon(recording? QStyle::SP_MediaStop: QStyle::SP_DialogNoButton));
}
