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

ChartWidget::ChartWidget(TcpSocket* socket, QWidget* parent): QWidget(parent), m_socket(socket)
{
    createSettings();
}

void ChartWidget::createSettings()
{
    auto tabWidget = new QTabWidget;
    tabWidget->addTab(tableSignals = new TableSignals, "信号检测列表");
    tabWidget->addTab(tableInterference = new TableInterference, "干扰信号测量记录");
    tabWidget->addTab(tableNoise = new TableNoise, "电磁信号人为噪声电平测量记录");
    auto doaLayout = new QVBoxLayout(zcWidget = new QWidget);
    auto hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(chartNB = new ChartNB(tr("窄带"), tr("Freq(MHz)"), MIN_FREQ, MAX_FREQ, tr("Power(dBm)"), MIN_AMPL, MAX_AMPL), 1);
    hBoxLayout->addWidget(tabWidget, 1);
    doaLayout->addLayout(hBoxLayout, 4);
    doaLayout->addWidget(chartWB = new ChartWB(tr("宽带"), tr("Freq(MHz)"), MIN_FREQ, MAX_FREQ, tr("Power(dBm)"), MIN_AMPL, MAX_AMPL), 5);

    auto toolBarLayout = new QVBoxLayout;
    auto style = QApplication::style();
    toolBarLayout->addWidget(continueBtn = new QPushButton(style->standardIcon(QStyle::SP_MediaPause), ""));
    toolBarLayout->addWidget(hideBtn = new QPushButton(style->standardIcon(QStyle::SP_ArrowLeft), ""));
    toolBarLayout->addWidget(clrBtn = new QPushButton(style->standardIcon(QStyle::SP_LineEditClearButton), ""));

    statusEdit = new SideWidget(hideBtn, clrBtn);
//    connect(m_socket, &TcpSocket::sendSocketStatus, statusEdit, &SideWidget::updateStatus);

    auto leftLayout = new QGridLayout;
    auto chartLayout = new QHBoxLayout;
    chartLayout->addWidget(zcWidget);
    leftLayout->addLayout(chartLayout, 0, 0);
    auto hLayout = new QHBoxLayout;
//    leftLayout->addLayout(hLayout, 1, 0);
//    leftLayout->setRowStretch(0, 3);
//    leftLayout->setRowStretch(1, 1);
    hLayout->addWidget(statusEdit, 1);
    hLayout->addLayout(toolBarLayout);

    freqListTable = new PointTableView(continueBtn);
    freqListTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    hLayout->addWidget(freqListTable, 1);

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

    auto parameterCheckBox = new QGroupBox(tr("Params Setting"));
    settingLayout->addRow(parameterCheckBox);
    auto parameterLayout = new QFormLayout(parameterCheckBox);

    freqResBox = new QComboBox();
    freqResBox->addItem("3.125KHz", 3.125);
    freqResBox->addItem("6.25KHz", 6.25);
    freqResBox->addItem("12.5KHz", 12.5);
    freqResBox->addItem("25KHz", 25);
    freqResBox->setCurrentIndex(0);
    parameterLayout->addRow(tr("Resolution:"), freqResBox);
    connect(freqResBox, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if(index < 0)
            return;
        g_parameter_set.FreqRes = freqResBox->itemData(index).toFloat();
        m_socket->parameter_set();
    });

    smoothBox = new QComboBox();
    smoothBox->addItem("1", 1);
    smoothBox->addItem("2", 2);
    smoothBox->addItem("4", 4);
    smoothBox->addItem("8", 8);
    smoothBox->addItem("16", 16);
    smoothBox->addItem("32", 32);
    parameterLayout->addRow(tr("Smooth:"), smoothBox);
    connect(smoothBox, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if(index < 0)
            return;
        g_parameter_set.SmNum = smoothBox->itemData(index).toUInt();
        m_socket->parameter_set();
    });

    gainModeBox = new QComboBox();
    gainModeBox->addItem("MGC", 0);
    gainModeBox->addItem("AGC-Fast", 1);
    gainModeBox->addItem("AGC-Normal", 2);
    gainModeBox->addItem("AGC-Slow", 3);
    gainModeBox->setEnabled(false);
    parameterLayout->addRow(tr("Gain Mode:"), gainModeBox);
    connect(gainModeBox, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if (index < 0)
            return;
        g_parameter_set.GMode = gainModeBox->itemData(index).toUInt();
        m_socket->parameter_set();
    });

    gainEdit = new QDoubleSpinBox;
    gainEdit->setMinimum(0);
    gainEdit->setMaximum(60);
    gainEdit->setSingleStep(1);
    gainEdit->setDecimals(0);
    gainEdit->setValue(0);
    parameterLayout->addRow(tr("Desc(0~60dB):"), gainEdit);
    connect(gainEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (gainEdit->hasFocus())
        {
            g_parameter_set.MGC = gainEdit->text().toUInt();
            m_socket->parameter_set();
        }
    });

    rcvModeBox = new QComboBox();
    rcvModeBox->addItem(tr("Normal"), 0);
//    rcvModeBox->addItem(tr("Low Distortion"), 1);
    rcvModeBox->addItem(tr("Low Noise"), 2);
    parameterLayout->addRow(tr("Receiver Work Mode:"), rcvModeBox);
    connect(rcvModeBox, QOverload<int>::of(&QComboBox::activated), this, [this](int) {
        g_parameter_set.RcvMode = rcvModeBox->currentData().toUInt();
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
