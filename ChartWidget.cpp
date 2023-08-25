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
    auto tabWidget = new QTabWidget;
    tabWidget->addTab(tableSignals = new TableSignals, "信号检测列表");
    tabWidget->addTab(tableInterference = new TableInterference, "干扰信号测量记录");
    tabWidget->addTab(tableNoise = new TableNoise, "电磁信号人为噪声电平测量记录");
    auto doaLayout = new QVBoxLayout(zcWidget = new QWidget);
    auto hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(chartNB, 1);
    hBoxLayout->addWidget(tabWidget, 1);
    doaLayout->addLayout(hBoxLayout, 4);
    doaLayout->addWidget(chartWB = new ChartWB(tr("宽带")), 5);

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

    CenterFreqEdit = new QDoubleSpinBox;
    CenterFreqEdit->setMinimum(MIN_FREQ);
    CenterFreqEdit->setMaximum(MAX_FREQ);
    CenterFreqEdit->setSingleStep(1);
    CenterFreqEdit->setDecimals(0);
    CenterFreqEdit->setValue(MID_FREQ);
    parameterLayout->addRow(tr("Center Freq(MHz):"), CenterFreqEdit);
    connect(CenterFreqEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (CenterFreqEdit->hasFocus())
        {
            g_parameter_set.CenterFreq = CenterFreqEdit->value() * 1e6;
            m_socket->parameter_set();
        }
    });

    simBWBox = new QComboBox();
    simBWBox->addItem("0.9375", 1);
    simBWBox->addItem("1.875", 2);
    simBWBox->addItem("3.75", 3);
    simBWBox->addItem("7.5", 4);
    simBWBox->addItem("15", 5);
    simBWBox->addItem("30", 6);
    simBWBox->setCurrentIndex(5);
    parameterLayout->addRow(tr("Sim Band(MHz):"), simBWBox);
    connect(simBWBox, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if (index < 0)
            return;
        for (int i = 0; i < 5; ++i)
        {
            freqResBox->setItemText(i, QString::number(RESOLUTIONS[index + i]) + "KHz");
        }
        g_parameter_set.FreqRes = freqResBox->currentData().toInt();
        g_parameter_set.SimBW = simBWBox->currentData().toInt();
        m_socket->parameter_set();
        //        auto bandWidth = simBWBox->itemData(index).toDouble();
        //        auto center = centerEdit->value(),
        //            min = center - bandWidth / 2,
        //            max = center + bandWidth / 2;
        //        dfChart->setAxisxMin(min);
        //        dfChart->setAxisxMax(max);
    });


    freqResBox = new QComboBox();
    auto index = simBWBox->currentIndex();
    for (auto i = 0; i < 5; ++i)
    {
        freqResBox->addItem(QString::number(RESOLUTIONS[index + i]), 14 - i);
    }
    freqResBox->setCurrentIndex(0);
    parameterLayout->addRow(tr("Resolution(kHz):"), freqResBox);
    connect(freqResBox, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if(index < 0)
            return;
        g_parameter_set.FreqRes = freqResBox->itemData(index).toInt();
        m_socket->parameter_set();
    });

    smoothBox = new QComboBox();
    for (auto i = 4; i <= 10; ++i)
    {
        smoothBox->addItem(QString::number(i), i);
    }
    parameterLayout->addRow(tr("Smooth:"), smoothBox);
    connect(smoothBox, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if (index < 0)
            return;
        g_parameter_set.SmNum = smoothBox->itemData(index).toUInt();
        m_socket->parameter_set();
    });

    RfGainModeGroup = new QButtonGroup(this);
    auto pLayout = new QHBoxLayout;
    for (int i = 0; i < 2; ++i)
    {
        static QString RadioText[] = { tr("MGC"), tr("AGC") };
        auto pButton = new QRadioButton(RadioText[i], this);
        pLayout->addWidget(pButton);
        RfGainModeGroup->addButton(pButton);
        RfGainModeGroup->setId(pButton, i);
    }
    parameterLayout->addRow(tr("Rf Gain Mode:"), pLayout);
    connect(RfGainModeGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton*) {
        g_parameter_set.GainMode = RfGainModeGroup->checkedId();
        m_socket->parameter_set();
    });

    RfGainEdit = new QDoubleSpinBox;
    RfGainEdit->setMinimum(0);
    RfGainEdit->setMaximum(60);
    RfGainEdit->setSingleStep(1);
    RfGainEdit->setDecimals(0);
    RfGainEdit->setValue(0);
    parameterLayout->addRow(tr("Rf Desc(0~31dB):"), RfGainEdit);
    connect(RfGainEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (RfGainEdit->hasFocus())
        {
            g_parameter_set.Rf_MGC = RfGainEdit->text().toUInt();
            m_socket->parameter_set();
        }
    });

    DigitGainEdit = new QDoubleSpinBox;
    DigitGainEdit->setMinimum(0);
    DigitGainEdit->setMaximum(60);
    DigitGainEdit->setSingleStep(1);
    DigitGainEdit->setDecimals(0);
    DigitGainEdit->setValue(0);
    parameterLayout->addRow(tr("Digit Desc(0~31dB):"), DigitGainEdit);
    connect(DigitGainEdit, &QDoubleSpinBox::editingFinished, this, [this] {
        if (DigitGainEdit->hasFocus())
        {
            g_parameter_set.Digit_MGC = DigitGainEdit->text().toUInt();
            m_socket->parameter_set();
        }
    });

    FeedbackGroup = new QButtonGroup(this);
    pLayout = new QHBoxLayout;
    for (int i = 0; i < 2; ++i)
    {
        static QString RadioText[] = { tr("Close"), tr("Open") };
        auto pButton = new QRadioButton(RadioText[i], this);
        pLayout->addWidget(pButton);
        FeedbackGroup->addButton(pButton);
        FeedbackGroup->setId(pButton, i);
    }
    parameterLayout->addRow(tr("Feedback State:"), pLayout);
    connect(FeedbackGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton*) {
        g_parameter_set.Feedback = FeedbackGroup->checkedId();
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
