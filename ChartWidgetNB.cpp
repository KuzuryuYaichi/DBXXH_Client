#include "ChartWidgetNB.h"

#include <QStyle>

ChartWidgetNB::ChartWidgetNB(QString title, int index, QWidget* parent): ChartWidgetCombine(title, parent), index(index)
{
    hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(new QLabel(tr("Domain:")), 1);
    hBoxLayout->addWidget(showBox = new QComboBox, 2);
    showBox->addItem(tr("Time"), WAVE_MODE);
    showBox->addItem(tr("Freq"), SPECTRUM_MODE);
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

    layoutSpectrum->addWidget(chartWave = new ChartViewWave(title, 0, DDC_LEN, SHRT_MIN, SHRT_MAX));
    layoutSpectrum->addWidget(chartSpectrum = new ChartViewSpectrumNB(title, MIN_FREQ, MAX_FREQ, MIN_AMPL, MAX_AMPL));

    connect(chartSpectrum->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](const QCPRange& newRange) {
        chartWaterfall->xAxis->setRange(newRange);
        chartWaterfall->replot();
    });
    connect(chartWaterfall->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), this, [this](const QCPRange& newRange) {
        chartSpectrum->xAxis->setRange(newRange);
        if (chartSpectrum->isVisible())
            chartSpectrum->replot();
    });

    chartSpectrum->hide();
    chartWave->show();

    connect(freqEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (freqEdit->hasFocus())
        {
            ParamsChange();
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
    boundBox->setCurrentIndex(boundBox->count() - 1);
    connect(boundBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
        ParamsChange();
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("CW(kHz):")), 1);
    hBoxLayout->addWidget(cwEdit = new QSpinBox, 2);
    connect(cwEdit, &QSpinBox::editingFinished, this, [this] {
        if (cwEdit->hasFocus())
        {
            ParamsChange();
        }
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Play:")), 1);
    auto style = QApplication::style();
    hBoxLayout->addWidget(playBtn = new QPushButton(style->standardIcon(QStyle::SP_MediaPlay), ""), 2);
    connect(playBtn, &QPushButton::clicked, this, [this] {
        emit triggerListening(this->index, playing = !playing);
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Rec:")), 1);
    hBoxLayout->addWidget(recordBtn = new QPushButton(style->standardIcon(QStyle::SP_DialogNoButton), ""), 2);
    connect(recordBtn, &QPushButton::clicked, this, [this] {
        changedRecording();
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(new QLabel(tr("Demod:")), 1);
    hBoxLayout->addWidget(demodBox = new QComboBox, 2);
    static constexpr const char* DEMOD_TYPE[] = { "IQ", "AM", "FM", "PM", "USB", "LSB", "ISB", "CW", "FSK", "PSK" };
    for (auto i = 0ull; i < 10; ++i)
        demodBox->addItem(DEMOD_TYPE[i], i);
    connect(demodBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
        auto demodState = demodBox->currentText() == "FSK";
        LblFSK->setVisible(demodState);
        RateEditFSK->setVisible(demodState);
        ParamsChange();
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(LblFSK = new QLabel(tr("FSK Rate(kHz):")));
    LblFSK->hide();
    hBoxLayout->addWidget(RateEditFSK = new QDoubleSpinBox);
    RateEditFSK->setMinimum(1);
    RateEditFSK->setMaximum(1000);
    RateEditFSK->setSingleStep(1);
    RateEditFSK->setDecimals(6);
    RateEditFSK->hide();

    inR = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * DDC_LEN);
    outR = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * DDC_LEN);
    planR = fftw_plan_dft_1d(DDC_LEN, inR, outR, FFTW_FORWARD, FFTW_MEASURE);
}

ChartWidgetNB::~ChartWidgetNB()
{
    fftw_destroy_plan(planR);
    fftw_free(inR);
    fftw_free(outR);
}

void ChartWidgetNB::fft(unsigned char* buf)
{
    auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
    auto data = (NarrowDDC*)(param + 1);
    static const auto WINDOW = HanningWindow<DDC_LEN>();
    for (auto p = 0; p < param->DataPoint; ++p)
    {
        inR[p][0] = data[p].I * WINDOW[p];
        inR[p][1] = data[p].Q * WINDOW[p];
    }
    fftw_execute(planR);
    const auto HALF_LEN = param->DataPoint / 2;
    for (int p = 0; p < HALF_LEN; ++p)
    {
        std::swap(outR[p], outR[HALF_LEN + p]);
    }
}

void ChartWidgetNB::ParamsChange()
{
    emit ParamsChanged(freqEdit->value() * 1e6, boundBox->currentData().toULongLong(), demodBox->currentData().toUInt(), cwEdit->value() * 1e3);
}

void ChartWidgetNB::changedListening(int index, bool state)
{
    if (this->index != index && state)
    {
        playing = false;
    }
    playBtn->setIcon(QApplication::style()->standardIcon(playing? QStyle::SP_MediaPause: QStyle::SP_MediaPlay));
}

void ChartWidgetNB::changedRecording()
{
    recordBtn->setIcon(QApplication::style()->standardIcon((recording = !recording)? QStyle::SP_MediaStop: QStyle::SP_DialogNoButton));
    if (recording)
    {
        std::lock_guard<std::mutex> lk(fileLock);
        file.setFileName(QString("Channel_%1 %2").arg(index + 1).arg(QDateTime::currentDateTime().toString("yyyy_MM_dd hh_mm_ss")));
    }
}

void ChartWidgetNB::Record(unsigned char* const buf)
{
    if (!recording)
        return;
    auto head = (DataHead*)buf;
    if (head->PackType != 0x602)
        return;
    auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
    auto amplData = (NarrowDDC*)(param + 1);

    switch (demodBox->currentIndex())
    {
    case FSK:
    {
        auto ptr = std::make_unique<short[]>(param->DataPoint);
        for (auto i = 0; i < param->DataPoint; ++i)
        {
            ptr[i] = amplData[i].I > 0? 1: 0;
        }
        WriteFile((char*)ptr.get(), sizeof(short) * param->DataPoint);
        break;
    }
    case PSK:
    {
        auto ptr = std::make_unique<NarrowDDC[]>(param->DataPoint);
        for (auto i = 0; i < param->DataPoint; ++i)
        {
            ptr[i].I = amplData[i].I > 0? 1: 0;
            ptr[i].Q = amplData[i].Q > 0? 1: 0;
        }
        WriteFile((char*)ptr.get(), sizeof(NarrowDDC) * param->DataPoint);
        break;
    }
    case AM:
    case FM:
    case PM:
    case USB:
    case LSB:
    {
        auto ptr = std::make_unique<short[]>(param->DataPoint);
        for (auto i = 0; i < param->DataPoint; ++i)
        {
            ptr[i] = amplData[i].I;
        }
        WriteFile((char*)ptr.get(), sizeof(short) * param->DataPoint);
        break;
    }
    case IQ:
    case ISB:
    {
        WriteFile((char*)amplData, sizeof(NarrowDDC) * param->DataPoint);
        break;
    }
    default: return;
    }
}

void ChartWidgetNB::WriteFile(char* buf, int length)
{
    std::lock_guard<std::mutex> lk(fileLock);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QMessageBox::warning(this, tr("Failed to Open file"), file.errorString());
        return;
    }
    int offset = 0;
    while (length > 0)
    {
        auto len = file.write(buf + offset, length);
        if (len > 0)
        {
            length -= len;
            offset += len;
        }
    }
    file.close();
}

void ChartWidgetNB::ChangeMode(int index)
{
    switch (index)
    {
    case WAVE_MODE:
    {
        chartSpectrum->hide();
        chartWave->show();
        break;
    }
    case SPECTRUM_MODE:
    {
        chartWave->hide();
        chartSpectrum->show();
        break;
    }
    }
}

void ChartWidgetNB::replace(unsigned char* const buf)
{
    fft(buf);
    Record(buf);
    auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
    switch (showBox->currentData().toInt())
    {
    case WAVE_MODE:
    {
        chartWave->replace(buf);
        break;
    }
    case SPECTRUM_MODE:
    {
        chartSpectrum->replace(buf, outR);
//        chartWaterfall->replace(buf, param->);
        break;
    }
    }
}
