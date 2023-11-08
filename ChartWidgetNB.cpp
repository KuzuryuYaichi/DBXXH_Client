#include "ChartWidgetNB.h"

#include "StructNetData.h"
#include "DigitDemod.h"

#include <QStyle>
#include <QStorageInfo>
#include <algorithm>

extern PARAMETER_SET g_parameter_set;

ChartWidgetNB::ChartWidgetNB(QString title, int index, QWidget* parent): ChartWidgetCombine(parent), index(index)
{
    hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(new QLabel(tr("Domain:")), 1);
    hBoxLayout->addWidget(showBox = new QComboBox, 2);
    showBox->addItem(tr("Time"), WAVE_MODE);
    showBox->addItem(tr("Freq"), SPECTRUM_MODE);
    showBox->addItem(tr("Waterfall"), WATERFALL_MODE);
    showBox->addItem(tr("Scatter"), SCATTER_MODE);
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
    mainLayout->addLayout(layoutSpectrum = new QHBoxLayout, 10);

    layoutSpectrum->addWidget(chartWave = new ChartViewWave(title, 0, DDC_LEN, SHRT_MIN, SHRT_MAX));
    layoutSpectrum->addWidget(chartSpectrum = new ChartViewSpectrumNB(title, MIN_FREQ, MAX_FREQ, MIN_AMPL, MAX_AMPL));
    layoutSpectrum->addWidget(chartWaterfall = new ChartViewWaterfall(title, MIN_FREQ, MAX_FREQ, -WATERFALL_DEPTH, 0));
    layoutSpectrum->addWidget(chartScatter = new ChartViewScatter(title, MIN_PHASE, MAX_PHASE, MIN_PHASE, MAX_PHASE));

    connect(chartSpectrum, &ChartViewSpectrumNB::RecordThresholdSignal, this, [this](double RecordThreshold) {
        this->RecordThreshold = RecordThreshold;
    });
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
    chartWaterfall->hide();
    chartScatter->hide();
    chartWave->show();

    connect(freqEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (freqEdit->hasFocus())
        {
            ParamsChange();
        }
    });

    hBoxLayout->addWidget(new QLabel(tr("BW(kHz):")), 1);
    hBoxLayout->addWidget(bandwidthBox = new QComboBox, 2);
    bandwidthBox->addItem("0.15", 150);
    bandwidthBox->addItem("0.3", 300);
    bandwidthBox->addItem("0.6", 600);
    bandwidthBox->addItem("1.5", 1500);
    bandwidthBox->addItem("2.4", 2400);
    bandwidthBox->addItem("6", 6000);
    bandwidthBox->addItem("9", 9000);
    bandwidthBox->addItem("15", 15000);
    bandwidthBox->addItem("30", 30000);
    bandwidthBox->addItem("50", 50000);
    bandwidthBox->addItem("120", 120000);
    bandwidthBox->addItem("150", 150000);
    bandwidthBox->setCurrentIndex(bandwidthBox->count() - 1);
    connect(bandwidthBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
        ChangeDigitDemodRate();
        ParamsChange();
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
    for (auto i = 0; i < 10; ++i)
    {
        demodBox->addItem(DEMOD_TYPE[i], i);
    }

    connect(demodBox, QOverload<int>::of(&QComboBox::activated), this, [this] (int) {
        auto State = demodBox->currentIndex() == FSK;
        LblFSK->setVisible(State);
        RateBoxFSK->setVisible(State);

        State = demodBox->currentIndex() == PSK;
        LblDQPSK->setVisible(State);
        RateBoxDQPSK->setVisible(State);

        State = demodBox->currentIndex() == AM;
        LblDepthAM->setVisible(State);
        DepthAM->setVisible(State);

        State = demodBox->currentIndex() == FM;
        LblMute->setVisible(State);
        TruncateSlider->setVisible(State);

        State = demodBox->currentIndex() == CW;
        LblCW->setVisible(State);
        cwEdit->setVisible(State);

        ParamsChange();
    });
    hBoxLayout->addStretch(1);

    hBoxLayout->addWidget(LblFSK = new QLabel(tr("FSK Rate(kHz):")));
    LblFSK->hide();
    hBoxLayout->addWidget(RateBoxFSK = new QComboBox);
    RateBoxFSK->hide();

    hBoxLayout->addWidget(LblDQPSK = new QLabel(tr("PSK Rate(kHz):")));
    LblDQPSK->hide();
    hBoxLayout->addWidget(RateBoxDQPSK = new QComboBox);
    RateBoxDQPSK->hide();
    connect(RateBoxDQPSK, QOverload<int>::of(&QComboBox::activated), this, [this](int) {
        ParamsChange();
    });

    ChangeDigitDemodRate();

    hBoxLayout->addWidget(LblDepthAM = new QLabel(tr("AM Depth(%):")));
    LblDepthAM->hide();
    hBoxLayout->addWidget(DepthAM = new QLabel("0"));
    DepthAM->hide();

    hBoxLayout->addWidget(LblMute = new QLabel(tr("Volume:")));
    LblMute->hide();
    hBoxLayout->addWidget(TruncateSlider = new QSlider(Qt::Horizontal));
    TruncateSlider->setTickInterval(6);
    TruncateSlider->setMinimum(19);
    TruncateSlider->setMaximum(39);
    TruncateSlider->setMinimumWidth(100);
    TruncateSlider->hide();
    connect(TruncateSlider, &QSlider::sliderReleased, this, [this] {
        ParamsChange();
    });

    hBoxLayout->addWidget(LblCW = new QLabel(tr("CW(kHz):")), 1);
    LblCW->hide();
    hBoxLayout->addWidget(cwEdit = new QSpinBox, 2);
    cwEdit->hide();
    connect(cwEdit, &QSpinBox::editingFinished, this, [this] {
        if (cwEdit->hasFocus())
        {
            ParamsChange();
        }
    });

    hBoxLayout->addWidget(showWaveBtn = new QPushButton(style->standardIcon(QStyle::SP_MediaPause), ""), 2);
    connect(showWaveBtn, &QPushButton::clicked, this, [this] {
        showWave = !showWave;
        showWaveBtn->setIcon(QApplication::style()->standardIcon(showWave? QStyle::SP_MediaPause: QStyle::SP_MediaPlay));
    });

    hBoxLayout->addWidget(Tmp_Chk = new QCheckBox());

    AmplData = std::make_unique<unsigned char[]>(DDC_LEN);
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

void ChartWidgetNB::ChangeDigitDemodRate()
{
    static constexpr int TIMES[] = { 4, 6, 8 };
    auto SpsBandwidth = NB_HALF_BANDWIDTH_KHz[bandwidthBox->currentIndex()];
    RateBoxFSK->clear();
    RateBoxDQPSK->clear();
    for (auto i = 0; i < 3; ++i)
    {
        auto Rate = SpsBandwidth / TIMES[i];
        RateBoxFSK->addItem(QString::number(Rate), Rate);
        RateBoxDQPSK->addItem(QString::number(Rate), Rate);
    }
}

void ChartWidgetNB::FFT(unsigned char* buf)
{
    auto param = (StructNBWave*)(buf + sizeof(DataHead));
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
    for (int p = 0; p < param->DataPoint; ++p)
    {
        AmplData[p] = 20 * std::log10(std::sqrt(std::pow(outR[p][0], 2.0) + std::pow(outR[p][1], 2.0))) + 42;
    }
}

void ChartWidgetNB::ParamsChange()
{
    emit ParamsChanged(freqEdit->value() * 1e6, bandwidthBox->currentData().toULongLong(), demodBox->currentData().toUInt(),
                       cwEdit->value() * 1e3, RateBoxDQPSK->currentData().toDouble() * 1e3, 48 - TruncateSlider->value());
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
    if (!recording)
    {
        auto Path = g_parameter_set.tinyConfig.Get_StoragePath();
        QDir dir;
        if (!dir.exists(Path))
            dir.mkpath(Path);
        std::lock_guard<std::mutex> lk(fileLock);
        file.setFileName(Path + "/" + QString("Channel_%1 %2.dat").arg(index + 1).arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")));
    }
    recordBtn->setIcon(QApplication::style()->standardIcon((recording = !recording)? QStyle::SP_MediaStop: QStyle::SP_DialogNoButton));
}

bool ChartWidgetNB::TestRecordThreshold()
{
    for (auto i = 0; i < DDC_LEN; ++i)
    {
        auto Ampl = (short)AmplData[i] + AMPL_OFFSET;
        if (Ampl > RecordThreshold)
        {
            return true;
        }
    }
    return false;
}

void ChartWidgetNB::Record(unsigned char* const buf)
{
    if (!(recording && TestRecordThreshold()))
        return;
    RemoveFile();
    auto param = (StructNBWave*)(buf + sizeof(DataHead));
    auto amplData = (NarrowDDC*)(param + 1);
    switch (demodBox->currentIndex())
    {
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
    case CW:
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
        if (len < 0)
            break;
        length -= len;
        offset += len;
    }
    file.close();
}

bool ChartWidgetNB::CheckStorage()
{
    auto Path = g_parameter_set.tinyConfig.Get_StoragePath();
    auto Threshold = g_parameter_set.tinyConfig.Get_StorageThreshold();
    auto storageInfoList = QStorageInfo::mountedVolumes();
    foreach (const auto& storage, storageInfoList)
    {
        if (Path.contains(storage.rootPath()))
        {
            qDebug() << storage.rootPath() << " " << storage.bytesAvailable();
            if (storage.bytesAvailable() < Threshold)
                return false;
        }
    }
    return true;
}

void ChartWidgetNB::RemoveFile()
{
    auto Path = g_parameter_set.tinyConfig.Get_StoragePath();
    auto Threshold = g_parameter_set.tinyConfig.Get_StorageThreshold();
    QDir dir(Path);
    if (!dir.exists())
        return;
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    dir.setNameFilters({ "*.dat" });
    dir.setSorting(QDir::Time | QDir::Reversed);
    auto fileList = dir.entryList();
    unsigned long long fileSize = 0;
    for (int i = 0; i < fileList.size(); ++i)
    {
        QFile file(Path + "/" + fileList[i]);
        fileSize += file.size();
    }
    for (int i = 0; i < fileList.size(); ++i)
    {
        if (fileSize < Threshold)
            break;
        QFile file(Path + "/" + fileList[i]);
        if (!file.exists())
            continue;
        auto size = file.size();
        if (!file.remove())
            break;
        fileSize -= size;
    }
}

void ChartWidgetNB::ChangeMode(int index)
{
    switch (index)
    {
    case WAVE_MODE:
    {
        chartSpectrum->hide();
        chartWaterfall->hide();
        chartScatter->hide();
        chartWave->show();
        break;
    }
    case SPECTRUM_MODE:
    {
        chartWave->hide();
        chartWaterfall->hide();
        chartScatter->hide();
        chartSpectrum->show();
        break;
    }
    case WATERFALL_MODE:
    {
        chartWave->hide();
        chartSpectrum->hide();
        chartScatter->hide();
        chartWaterfall->show();
        break;
    }
    case SCATTER_MODE:
    {
        chartWave->hide();
        chartSpectrum->hide();
        chartWaterfall->hide();
        chartScatter->show();
        break;
    }
    }
}

void ChartWidgetNB::replace(const std::shared_ptr<unsigned char[]>& data)
{
    if (!showWave)
        return;
    auto buf = data.get();

    static std::unique_ptr<unsigned char[]> FSK_Data = nullptr;
    //Especially For ISB
    auto param = (StructNBWave*)(buf + sizeof(DataHead));
    auto amplData = (NarrowDDC*)(param + 1);
    if (demodBox->currentIndex() == ISB)
    {
        for (auto i = 0; i < param->DataPoint; ++i)
        {
            std::swap(amplData[i].I, amplData[i].Q);
        }
    }
    else if (demodBox->currentIndex() == FSK)
    {
        if (Tmp_Chk->checkState())
        {
            FSK_Data = DigitDemod::Demod(data, RateBoxFSK->currentData().toDouble());
            if (FSK_Data == nullptr)
                return;
            buf = FSK_Data.get();
        }
    }

    FFT(buf);
    Record(buf);
    switch (showBox->currentData().toInt())
    {
    case WAVE_MODE:
    {
        chartWave->replace(buf);
        break;
    }
    case SPECTRUM_MODE:
    {
        chartSpectrum->replace(buf, AmplData.get());
        break;
    }
    case SCATTER_MODE:
    {
        chartScatter->replace(buf);
        break;
    }
    }
    chartWaterfall->replace(buf, AmplData.get());

    if (!ready)
        return;
    ready = false;

    std::thread([this, data] {
        QTimer::singleShot(0, this, [this, data] {
            auto param = (StructNBWave*)(data.get() + sizeof(DataHead));
            if (param->DataType == AM && param->AM_DC != 0)
            {
                auto depth = (char)(100.0 * param->AM_DataMax / param->AM_DC);
                if (depth >= 0 && depth <= 100)
                    DepthAM->setText(QString::number(depth));
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(REFRESH_INTERVAL));
        ready = true;
    }).detach();
}
