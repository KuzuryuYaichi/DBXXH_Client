#include "ChartNB.h"

#include <QStyle>

ChartNB::ChartNB(QString title, QString X_title, int AXISX_MIN, int AXISX_MAX, QString Y_title, int AXISY_MIN, int AXISY_MAX, QWidget* parent):
    CombineWidget(title, X_title, AXISX_MIN, AXISX_MAX, Y_title, AXISY_MIN, AXISY_MAX, parent)
{
    layoutAdjust->setVisible(true);
    auto HBoxLayout = new QHBoxLayout(layoutAdjust);
    HBoxLayout->addWidget(new QLabel(tr("NB Freq(MHz):")), 1);
    HBoxLayout->addWidget(freqEdit = new QDoubleSpinBox, 2);
    freqEdit->setMinimum(MIN_FREQ);
    freqEdit->setMaximum(MAX_FREQ);
    freqEdit->setSingleStep(1);
    freqEdit->setDecimals(3);
    connect(freqEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (freqEdit->hasFocus())
        {
//            emit FreqBandwidthChanged(freqEdit->value() * 1e3, bandBox->currentData().toULongLong());
//            ChangeAxis();
        }
    });
    HBoxLayout->addStretch(1);
    HBoxLayout->addWidget(new QLabel(tr("BW(kHz):")), 1);
    HBoxLayout->addWidget(bandBox = new QComboBox, 2);
    bandBox->addItem("2.4", 2400);
    bandBox->addItem("4.8", 4800);
    bandBox->addItem("9.6", 9600);
    bandBox->addItem("19.2", 19200);
    bandBox->addItem("38.4", 38400);
    bandBox->addItem("76.8", 76800);
    bandBox->addItem("96", 96000);
    connect(bandBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
//        emit FreqBandwidthChanged(freqEdit->value() * 1e3, bandBox->currentData().toULongLong());
//        ChangeAxis();
    });
    HBoxLayout->addStretch(1);
    HBoxLayout->addWidget(new QLabel(tr("Mode:")), 1);
    HBoxLayout->addWidget(showBox = new QComboBox, 2);
    showBox->addItem(tr("Time Domain"), DDC_MODE);
    showBox->addItem(tr("Freq Domain"), FFT_MODE);
    connect(showBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
//        ChangeAxis();
    });
    HBoxLayout->addStretch(1);
    HBoxLayout->addWidget(new QLabel(tr("监听:")), 1);
    auto style = QApplication::style();
    HBoxLayout->addWidget(playBtn = new QPushButton(style->standardIcon(QStyle::SP_MediaPlay), ""), 2);
    connect(playBtn, &QPushButton::clicked, this, [this] {
        changedListening(playing);
        emit triggerListening(playing = !playing);
    });
}

void ChartNB::changedListening(bool playing)
{
    this->playing = playing;
    auto style = QApplication::style();
    playBtn->setIcon(style->standardIcon(playing? QStyle::SP_MediaPlay: QStyle::SP_MediaPause));
}
