#include "ChartWidget.h"
#include <QGroupBox>
#include <QPainter>
#include <QObject>
#include <QDateTime>
#include <QThread>
#include <QApplication>
#include <QHeaderView>
#include <QStyle>
#include <QFormLayout>

extern PARAMETER_SET g_parameter_set;

ChartWidget::ChartWidget(TcpSocket* socket, ChartNB* chartNB, QWidget* parent): QWidget(parent), chartNB(chartNB), m_socket(socket)
{
    createSettings();
}

void ChartWidget::createSettings()
{
    auto doaLayout = new QVBoxLayout(zcWidget = new QWidget);
    auto hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(chartNB, 1);
    hBoxLayout->addWidget(wBSignalDetectWidget = new WBSignalDetectWidget, 1);
    doaLayout->addLayout(hBoxLayout, 4);
    doaLayout->addWidget(chartWB = new ChartWB(tr("宽带")), 5);

    statusEdit = new SideWidget;
//    connect(m_socket, &TcpSocket::sendSocketStatus, statusEdit, &SideWidget::updateStatus);

    auto leftLayout = new QGridLayout;
    auto chartLayout = new QHBoxLayout;
    chartLayout->addWidget(zcWidget);
    leftLayout->addLayout(chartLayout, 0, 0);
//    auto hLayout = new QHBoxLayout;
//    hLayout->addWidget(statusEdit, 1);

    auto settingLayout = new QFormLayout;
    settingLayout->setSpacing(9);

    auto tmpLayout = new QHBoxLayout;
    tmpLayout->addLayout(leftLayout, 3);

    auto receiveCheckBox = new QGroupBox(tr("Receiver Settings"));
    settingLayout->addRow(receiveCheckBox);
    auto receiveLayout = new QFormLayout(receiveCheckBox);

    auto checkBox = new QPushButton(tr("All Check"));
    receiveLayout->addRow(tr("Device SelfCheck:"), checkBox);
    connect(checkBox, &QAbstractButton::clicked, this, [this](bool) {
        m_socket->self_check(allCheck);
    });

    auto workCtrlBtn = new QPushButton(tr("Work Param Query"));
    receiveLayout->addRow(tr("Work Ctrl:"), workCtrlBtn);
    connect(workCtrlBtn, &QPushButton::clicked, this, [this] {
        m_socket->work_ctrl(GetParameter);
    });

    connect(chartWB, &ChartWB::ParamsChanged, this, [this] {
        m_socket->parameter_set();
    });

    auto connectBox = new QGroupBox(tr("Network"));
    settingLayout->addRow(connectBox);
    auto connectLayout = new QFormLayout(connectBox);

    ipEdit = new QLineEdit(g_parameter_set.tinyConfig.Get_DataIP());
    ipEdit->setAlignment(Qt::AlignCenter);
    connectLayout->addRow("Server IP:", ipEdit);
    portEdit = new QLineEdit(QString::number(g_parameter_set.tinyConfig.Get_DataPort()));
    portEdit->setAlignment(Qt::AlignCenter);
    connectLayout->addRow("Server Port:", portEdit);
    auto connectBtn = new QPushButton(tr("Connect"), this);
    connect(connectBtn, &QPushButton::clicked, this, [this]
    {
        auto ec = m_socket->connectToServer(ipEdit->text().toStdString(), portEdit->text().toUInt());
        if (ec.failed())
        {
            statusEdit->updateStatus(tr("Failed to Connect To Server: ") + QString::fromLocal8Bit(ec.what()));
        }
        else
        {
            statusEdit->updateStatus(tr("Connected To Server"));
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
