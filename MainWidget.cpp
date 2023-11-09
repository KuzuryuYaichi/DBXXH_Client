#include "MainWidget.h"
#include <QGroupBox>
#include <QPainter>
#include <QObject>
#include <QDateTime>
#include <QThread>
#include <QApplication>
#include <QStyle>
#include <QFormLayout>
#include <QPalette>

extern PARAMETER_SET g_parameter_set;

MainWidget::MainWidget(TcpSocket* socket, ChartWidgetNB** chartNBs, QWidget* parent): QWidget(parent), chartNBs(chartNBs), chartNB(chartNBs[0]), m_socket(socket)
{
    auto doaLayout = new QVBoxLayout(zcWidget = new QWidget);
    auto hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(chartNB, 1);
    hBoxLayout->addWidget(wbSignalDetectWidget = new WBSignalDetectWidget, 1);
    doaLayout->addLayout(hBoxLayout, 4);
    doaLayout->addWidget(chartWB = new ChartWidgetWB(tr("WB")), 7);
    connect(chartWB, &ChartWidgetWB::ParamsChanged, this, [this] {
        m_socket->wb_parameter_set();
    });
    connect(chartWB->chartSpectrum, &ChartViewSpectrumWB::triggerMark, this, [this](std::vector<std::tuple<bool, double, double>> MarkData) {
        this->MarkData = std::move(MarkData);
    });
    connect(chartWB->chartSpectrum, &ChartViewSpectrumWB::triggerRefStatus, this, [this](short mode) {
        this->RefStatusLbl->setText(mode? tr("Inner"): tr("Outer"));
    });
    connect(chartWB->chartSpectrum, &ChartViewSpectrumWB::triggerFreqNB, this, [this](int channel, double freq) {
        this->chartNBs[channel]->ChangeFreq(freq);
    });
    //    connect(m_socket, &TcpSocket::sendSocketStatus, statusEdit, &SideWidget::updateStatus);

    auto leftLayout = new QGridLayout;
    auto chartLayout = new QHBoxLayout;
    chartLayout->addWidget(zcWidget);
    leftLayout->addLayout(chartLayout, 0, 0);

    auto settingLayout = new QFormLayout;
    settingLayout->setSpacing(9);

    auto tmpLayout = new QHBoxLayout;
    tmpLayout->addLayout(leftLayout, 3);

    auto markersGroupBox = new QGroupBox(tr("Freq Marker"));
    settingLayout->addRow(markersGroupBox);
    auto markersLayout = new QFormLayout(markersGroupBox);
    for (auto i = 0; i < MARKER_NUM; ++i)
    {
        QPalette palette;
        palette.setColor(QPalette::WindowText, MARKER_COLOR[i]);
        auto markerGroupBox = new QGroupBox(tr("Marker %1").arg(i + 1));
        markerGroupBox->setPalette(palette);
        markersLayout->addRow(markerGroupBox);
        auto markerLayout = new QFormLayout(markerGroupBox);
        markerLayout->addRow(tr("MaxFreq"), MarkerFreqLbl[i] = new QLabel("-"));
        markerLayout->addRow(tr("MaxAmpl"), MarkerAmplLbl[i] = new QLabel("-"));
    }
    markersLayout->addRow(tr("Spectrum"), saveBtn = new QPushButton(tr("Save")));
    connect(saveBtn, &QPushButton::clicked, this, [this] {
        chartWB->chartSpectrum->SaveSpectrum(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".png");
    });

    auto TrackMeasureGroupBox = new QGroupBox(tr("Track Measure"));
    settingLayout->addRow(TrackMeasureGroupBox);
    auto TrackMeasureLayout = new QFormLayout(TrackMeasureGroupBox);
    TrackMeasureLayout->addRow(tr("Measure"), MeasureLbl = new QLabel("-"));
    connect(chartWB->chartSpectrum, &ChartViewSpectrumWB::triggerMeasure, this, [this](double Distance) {
        this->Distance = Distance;
    });
    TrackMeasureLayout->addRow(tr("MaxFreq"), MaxFreqLbl = new QLabel("-"));
    TrackMeasureLayout->addRow(tr("MaxAmpl"), MaxAmplLbl = new QLabel("-"));
    connect(chartWB->chartSpectrum, &ChartViewSpectrumWB::triggerTrack, this, [this](double MaxFreq, double MaxAmpl) {
        this->MaxFreq = MaxFreq;
        this->MaxAmpl = MaxAmpl;
    });

    auto FM_IndexGroupBox = new QGroupBox(tr("FM Index"));
    settingLayout->addRow(FM_IndexGroupBox);
    auto frequecyLayout = new QFormLayout(FM_IndexGroupBox);
    frequecyLayout->addRow(tr("FM_Freq(kHz)"), FM_FreqEdit = new QDoubleSpinBox);
    FM_FreqEdit->setMinimum(0.1);
    FM_FreqEdit->setValue(1);
    connect(FM_FreqEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (FM_FreqEdit->hasFocus() && FM_FreqEdit->value())
        {
            FM_IndexLbl->setText(QString("%1").arg(FM_Offset * 1e3 / FM_FreqEdit->value()));
        }
    });
    frequecyLayout->addRow(tr("FM_Index"), FM_IndexLbl = new QLabel("0"));
    connect(chartWB->chartSpectrum, &ChartViewSpectrumWB::triggerFM_Index, this, [this](double FM_Offset) {
        this->FM_Offset = FM_Offset;
    });

    auto Rf_StatusGroupBox = new QGroupBox(tr("Rf Status"));
    settingLayout->addRow(Rf_StatusGroupBox);
    auto Rf_StatusLayout = new QFormLayout(Rf_StatusGroupBox);
    Rf_StatusLayout->addRow(tr("Selfcheck"), selfcheckBtn = new QPushButton(tr("Query")));
    Rf_StatusLayout->addRow(tr("RefStatus"), RefStatusLbl = new QLabel(tr("Unknown")));
    connect(selfcheckBtn, &QPushButton::clicked, this, [this](bool) {
        m_socket->self_check();
    });

    m_updater = new QTimer;
    m_updater->setInterval(100);
    connect(m_updater, &QTimer::timeout, this, [this] {
        if (MarkData.size() == MARKER_NUM)
        {
            for (auto i = 0; i < MARKER_NUM; ++i)
            {
                if (std::get<0>(MarkData[i]))
                {
                    MarkerFreqLbl[i]->setText(QStringLiteral("%1MHz").arg(std::get<1>(MarkData[i])));
                    MarkerAmplLbl[i]->setText(QStringLiteral("%1dBm").arg(std::get<2>(MarkData[i])));
                }
                else
                {
                    MarkerFreqLbl[i]->setText("-");
                    MarkerAmplLbl[i]->setText("-");
                }
            }
            MeasureLbl->setText(QStringLiteral("%1MHz").arg(Distance));
            MaxFreqLbl->setText(QStringLiteral("%1MHz").arg(MaxFreq));
            MaxAmplLbl->setText(QStringLiteral("%1dBm").arg(MaxAmpl));
            FM_IndexLbl->setText(QString("%1").arg(FM_Offset * 1e3 / FM_FreqEdit->value()));
        }
    });
    m_updater->start();

    auto connectBox = new QGroupBox(tr("Network"));
    settingLayout->addRow(connectBox);
    auto connectLayout = new QFormLayout(connectBox);

    ipEdit = new QLineEdit(g_parameter_set.tinyConfig.Get_DataIP());
    ipEdit->setAlignment(Qt::AlignCenter);
    connectLayout->addRow(tr("Server IP:"), ipEdit);
    portEdit = new QLineEdit(QString::number(g_parameter_set.tinyConfig.Get_DataPort()));
    portEdit->setAlignment(Qt::AlignCenter);
    connectLayout->addRow(tr("Server Port:"), portEdit);
    auto connectBtn = new QPushButton(tr("Connect"), this);
    connect(connectBtn, &QPushButton::clicked, this, [this]
    {
        auto ec = m_socket->connectToServer(ipEdit->text().toStdString(), portEdit->text().toUInt());
        if (ec.failed())
        {
            //            statusEdit->updateStatus(tr("Failed to Connect To Server: ") + QString::fromLocal8Bit(ec.what()));
        }
        else
        {
            //            statusEdit->updateStatus(tr("Connected To Server"));
        }
    });
    connectLayout->addRow(connectBtn);

    auto mainGridLayout = new QGridLayout(this);
    mainGridLayout->addLayout(tmpLayout, 0, 0);
    mainGridLayout->addLayout(settingLayout, 0, 1);
    mainGridLayout->setSpacing(2);
    mainGridLayout->setColumnStretch(0, 10);
    mainGridLayout->setColumnStretch(1, 1);
}
