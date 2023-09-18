#include "MainWidget.h"
#include <QGroupBox>
#include <QPainter>
#include <QObject>
#include <QDateTime>
#include <QThread>
#include <QApplication>
#include <QHeaderView>
#include <QStyle>
#include <QFormLayout>
#include <QPalette>

extern PARAMETER_SET g_parameter_set;

MainWidget::MainWidget(TcpSocket* socket, ChartWidgetNB* chartNB, QWidget* parent): QWidget(parent), chartNB(chartNB), m_socket(socket)
{
    createSettings();
}

void MainWidget::createSettings()
{
    auto doaLayout = new QVBoxLayout(zcWidget = new QWidget);
    auto hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(chartNB, 1);
    hBoxLayout->addWidget(wBSignalDetectWidget = new WBSignalDetectWidget, 1);
    doaLayout->addLayout(hBoxLayout, 4);
    doaLayout->addWidget(chartWB = new ChartWidgetWB(tr("WB")), 5);
    connect(chartWB, &ChartWidgetWB::ParamsChanged, this, [this] {
        m_socket->wb_parameter_set();
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

    auto markerGroupBox = new QGroupBox(tr("Freq Marker"));
    settingLayout->addRow(markerGroupBox);
    auto markerLayout = new QFormLayout(markerGroupBox);
    for (auto i = 0; i < 5; ++i)
    {
        auto nameLbl = new QLabel(tr("Marker %1").arg(i + 1));
        QPalette palette;
        palette.setColor(QPalette::WindowText, MARKER_COLOR[i]);
        nameLbl->setPalette(palette);
        markerLayout->addRow(nameLbl, MarkerLbl[i] = new QLabel("-"));
    }
    connect(chartWB->chartSpectrum, &ChartViewSpectrum::triggerMark, this, [this](std::vector<std::pair<bool, double>> MarkAmpl) {
        this->MarkAmpl = std::move(MarkAmpl);
    });

    auto measureGroupBox = new QGroupBox(tr("Freq Measure"));
    settingLayout->addRow(measureGroupBox);
    auto measureLayout = new QFormLayout(measureGroupBox);
    measureLayout->addRow(tr("Measure"), MeasureLbl = new QLabel("-"));
    connect(chartWB->chartSpectrum, &ChartViewSpectrum::triggerMeasure, this, [this](double Distance) {
        this->Distance = Distance;
    });

    auto trackGroupBox = new QGroupBox(tr("Freq Track"));
    settingLayout->addRow(trackGroupBox);
    auto trackLayout = new QFormLayout(trackGroupBox);
    trackLayout->addRow(tr("Track"), TrackLbl = new QLabel("-"));
    connect(chartWB->chartSpectrum, &ChartViewSpectrum::triggerTrack, this, [this](QString MaxPoint) {
        this->MaxPoint = MaxPoint;
    });

    m_updater = new QTimer;
    m_updater->setInterval(100);
    m_updater->setSingleShot(true);
    connect(m_updater, &QTimer::timeout, this, [this] {
        if (MarkAmpl.size() < MARKER_NUM)
            return;
        for (auto i = 0; i < MARKER_NUM; ++i)
        {
            MeasureLbl[i].setText(MarkAmpl[i].first? QStringLiteral("%1dBm").arg(MarkAmpl[i].second): "-");
        }
        MeasureLbl->setText(QStringLiteral("%1MHz").arg(Distance));
        TrackLbl->setText(MaxPoint);
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
