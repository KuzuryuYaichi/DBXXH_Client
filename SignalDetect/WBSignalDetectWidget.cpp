#include "SignalDetect/WBSignalDetectWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QAxWidget>
#include <QAxObject>
#include "global.h"
#include "QXlsx/xlsxdocument.h"

WBSignalDetectWidget::WBSignalDetectWidget(QWidget *parent): QWidget(parent)
{
    setupUi();

    connect(this, &WBSignalDetectWidget::DetectSwitch, m_pSignalDetectModel, &WBSignalDetectModel::SetTime);
    connect(this, &WBSignalDetectWidget::DetectSwitch, m_pManMadeNoiseTable->m_pManMadeNoiseModel, &WBSignalDetectModel::SetTime);

    connect(m_pPopupParamDialog = new PopupParamDialog, &PopupParamDialog::sigUpdateParam, this, [this](const ParamSet& param) {
        m_pSignalDetectModel->setBandwidthThreshold(param.BandwidthThreshold);
        m_pSignalDetectModel->setFreqPointThreshold(param.FreqPointThreshold);
        m_pSignalDetectModel->setAmplThreshold(param.AmplThreshold);
    });
    m_pPopupParamDialog->setModal(false);

    connect(m_pTypicalFreqDialog = new TypicalFreqDialog, &TypicalFreqDialog::sigHaveTypicalFreq, this, [this](const std::list<int>& mapValue) {
        m_pManMadeNoiseTable->m_pManMadeNoiseModel->setLstTypicalFreq(mapValue);
    });
    m_pTypicalFreqDialog->setModal(false);

    connect(m_CommonInfoDialog = new CommonInfoDialog, &CommonInfoDialog::triggerSetCompleted, this, [this](const CommonInfoSet& CommonInfo) {
        m_CommonInfo = CommonInfo;
    });
    m_CommonInfoDialog->setModal(false);

    connect(m_ResistivityDialog = new ResistivityDialog, &ResistivityDialog::triggerSetCompleted, this, [this](const ResistivitySet& Resistivity) {
        GenerateExcelResistivityTable(m_CommonInfo, Resistivity);
    });
    m_ResistivityDialog->setModal(false);

    connect(m_ConductivityDialog = new ConductivityDialog, &ConductivityDialog::triggerSetCompleted, this, [this](const ConductivitySet& Conductivity) {
        GenerateExcelConductivityTable(m_CommonInfo, Conductivity);
    });
    m_ConductivityDialog->setModal(false);
}

void WBSignalDetectWidget::sigTriggerSignalDetect(unsigned char* amplData, int length, int StartFreq, int BandWidth)
{
    auto FFtin = ippsMalloc_32f(length);
    for (int i = 0; i < length; ++i)
    {
        FFtin[i] = (short)amplData[i] + AMPL_OFFSET;
    }
    m_pSignalDetectModel->findPeakCyclically(FFtin, length, StartFreq, BandWidth);
    m_pManMadeNoiseTable->m_pManMadeNoiseModel->findNoiseCharaAroundTypicalFreq(FFtin, length, StartFreq, BandWidth);
    ippsFree(FFtin);
}

void WBSignalDetectWidget::PulseDetect(Pulse* pulse, int len)
{
    m_pPulseDetectTable->m_pPulseDetectModel->replace(pulse, len);
}

void WBSignalDetectWidget::setupUi()
{
    connect(this, &WBSignalDetectWidget::DetectSwitch, this, [this](bool isDetecting) {
        pushButton_TypicalFreqSet->setVisible(!isDetecting);
    });

    auto mainLayout = new QVBoxLayout(this);
    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(pushButton_ParamSet = new QPushButton("门限检测"));
    pushButton_ParamSet->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogInfoView));
    connect(pushButton_ParamSet, &QPushButton::clicked, this, [this] {
        m_pPopupParamDialog->setModal(true);
        m_pPopupParamDialog->show();
    });
    horizontalLayout->addWidget(pushButton_TypicalFreqSet = new QPushButton("典型频点"));
    pushButton_TypicalFreqSet->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogInfoView));
    connect(pushButton_TypicalFreqSet, &QPushButton::clicked, this, [this] {
        m_pTypicalFreqDialog->setModal(true);
        m_pTypicalFreqDialog->SetCurrentTypicalFreqFromTable(m_pManMadeNoiseTable->m_pManMadeNoiseModel->lstTypicalFreq());
        m_pTypicalFreqDialog->show();
    });
    horizontalLayout->addWidget(pushButton_cleanAllData = new QPushButton("清理"));
    pushButton_cleanAllData->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogDiscardButton));
    connect(pushButton_cleanAllData, &QPushButton::clicked, this, [this] {
        m_pSignalDetectModel->CleanUp();
    });
    horizontalLayout->addStretch();

    horizontalLayout->addWidget(pushButton_Detect = new QPushButton("开始检测"));
    pushButton_Detect->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogYesButton));
    pushButton_Detect->setCheckable(true);
    connect(pushButton_Detect, &QPushButton::clicked, this, [this](bool isDetecting) {
        emit DetectSwitch(isDetecting);
        pushButton_Detect->setText(isDetecting? "停止检测": "开始检测");
        pushButton_Detect->setIcon(QApplication::style()->standardIcon(isDetecting? QStyle::SP_BrowserStop: QStyle::SP_DialogYesButton));
    });
    horizontalLayout->addWidget(pushButton_importLegal = new QPushButton("导入非法频点"));
    connect(pushButton_importLegal, &QPushButton::clicked, this, [this] {
        QMessageBox::information(nullptr, "导入非法频点", m_pSignalDetectModel->ImportLegalFreqConf()? "导入成功！": "导入失败！");
    });
    horizontalLayout->addWidget(pushButton_ExportLegal = new QPushButton("导出非法频点"));
    connect(pushButton_ExportLegal, &QPushButton::clicked, this, [this] {
        QMessageBox::information(nullptr, "导出非法频点", m_pSignalDetectModel->ExportLegalFreqConf()? "导出成功！": "导出失败！");
    });
    horizontalLayout->addWidget(pushButton_setLegalFreq = new QPushButton("设置非法频点"));
    pushButton_setLegalFreq->setCheckable(true);
    connect(pushButton_setLegalFreq, &QPushButton::clicked, this, [this](bool checked) {
        pushButton_setLegalFreq->setText(checked? "完成设置": "设置非法频点");
        if (m_pSignalDetectModel->m_eUserViewType == SIGNAL_DETECT_TABLE)
            m_pSignalDetectModel->TriggerLegalFreqSet(checked);
    });
    horizontalLayout->addWidget(pushButton_setRemark = new QPushButton("添加说明"));
    pushButton_setRemark->setCheckable(true);
    pushButton_setRemark->hide();
    connect(pushButton_setRemark, &QPushButton::clicked, this, [this](bool checked) {
        pushButton_setRemark->setText(checked? "完成设置": "添加说明");
        if (m_pSignalDetectModel->m_eUserViewType == DISTURB_NOISE_TABLE)
            m_pSignalDetectModel->TriggerRemarkSet(checked);
    });

    mainLayout->addLayout(horizontalLayout);
    mainLayout->addWidget(tabWidget_SignalDetectTable = new QTabWidget);
    tabWidget_SignalDetectTable->addTab(m_pSignalDetectTable = new SignalDetectTableView, "信号检测表");
    m_pSignalDetectTable->setModel(m_pSignalDetectModel = new SignalNoiseModel);
    tabWidget_SignalDetectTable->addTab(m_pDisturbNoiseTable = new DisturbNoiseTableView, "干扰信号测量表");
    m_pDisturbNoiseTable->setModel(m_pSignalDetectModel);
    tabWidget_SignalDetectTable->addTab(m_pManMadeNoiseTable = new ManMadeNoiseTableView, "电磁环境人为噪声电平测量表");
    tabWidget_SignalDetectTable->addTab(m_pPulseDetectTable = new PulseDetectTableView, "脉冲检测表");
    connect(tabWidget_SignalDetectTable, &QTabWidget::currentChanged, this, [this] (int index) {
        emit pushButton_setLegalFreq->clicked(false);
        emit pushButton_setRemark->clicked(false);
        switch (index)
        {
        case 0: m_pSignalDetectModel->setUserViewType(SIGNAL_DETECT_TABLE); break;
        case 1: m_pSignalDetectModel->setUserViewType(DISTURB_NOISE_TABLE); break;
        }
        pushButton_setLegalFreq->setVisible(index == SIGNAL_DETECT_TABLE);
        pushButton_setRemark->setVisible(index == DISTURB_NOISE_TABLE);
        pushButton_cleanAllData->setVisible(index == SIGNAL_DETECT_TABLE || index == DISTURB_NOISE_TABLE);
        m_pSignalDetectModel->UpdateData();
    });

    horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(pushButton_CommonInfo = new QPushButton("报告基本信息"));
    pushButton_CommonInfo->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogInfoView));
    connect(pushButton_CommonInfo, &QPushButton::clicked, this, [this]
    {
        m_CommonInfoDialog->setModal(true);
        m_CommonInfoDialog->show();
    });
    horizontalLayout->addStretch();

    horizontalLayout->addWidget(pushButton_GenerateSignalDetect = new QPushButton("信号检测"));
    pushButton_GenerateSignalDetect->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(pushButton_GenerateSignalDetect, &QPushButton::clicked, this, [this]
    {
        tabWidget_SignalDetectTable->setCurrentIndex(0);
        m_pSignalDetectModel->GenerateExcelSignalDetectTable();
    });
    horizontalLayout->addWidget(pushButton_GenerateDisturbSignal = new QPushButton("干扰信号"));
    pushButton_GenerateDisturbSignal->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(pushButton_GenerateDisturbSignal, &QPushButton::clicked, this, [this]
    {
        tabWidget_SignalDetectTable->setCurrentIndex(1);
        m_pSignalDetectModel->GenerateExcelDisturbNoiseTable(m_CommonInfo);
    });
    horizontalLayout->addWidget(pushButton_GenerateManMadeNoise = new QPushButton("电磁环境人为噪声"));
    pushButton_GenerateManMadeNoise->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(pushButton_GenerateManMadeNoise, &QPushButton::clicked, this, [this]
    {
        tabWidget_SignalDetectTable->setCurrentIndex(2);
        m_pManMadeNoiseTable->m_pManMadeNoiseModel->GenerateExcelManMadeNoiseTable(m_CommonInfo);
    });
    horizontalLayout->addWidget(pushButton_GenerateResistivity = new QPushButton("土壤电阻率"));
    pushButton_GenerateResistivity->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(pushButton_GenerateResistivity, &QPushButton::clicked, this, [this]
    {
        m_ResistivityDialog->setModal(true);
        m_ResistivityDialog->show();
    });
    horizontalLayout->addWidget(pushButton_GenerateConductivity = new QPushButton("土壤导电均匀性"));
    pushButton_GenerateConductivity->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(pushButton_GenerateConductivity, &QPushButton::clicked, this, [this]
    {
        m_ConductivityDialog->setModal(true);
        m_ConductivityDialog->show();
    });
    horizontalLayout->addWidget(pushButton_GenerateElecEnvReport = new QPushButton(REPORT_NAME));
    pushButton_GenerateElecEnvReport->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(pushButton_GenerateElecEnvReport, &QPushButton::clicked, this, [this] {
        QFileDialog dialog;
        auto folderName = dialog.getExistingDirectory(nullptr, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (folderName.isEmpty())
        {
            QMessageBox::critical(nullptr, "电磁环境测试报告", "路径不存在");
            return;
        }

        QAxWidget word("Word.Application", 0, Qt::MSWindowsOwnDC);
        word.setProperty("Visible", false);
        auto documents = word.querySubObject("Documents");
        documents->dynamicCall("Add(QString)", QDir::currentPath() + "/ReportFormat.docx");
        auto document = word.querySubObject("ActiveDocument");
        if (!document || document->isNull())
            return;
        if (!m_pManMadeNoiseTable->m_pManMadeNoiseModel->GenerateWordManMadeNoiseChart(document))
            return;
        document->dynamicCall("SaveAs(const QString&))", QDir::toNativeSeparators(folderName + "/电磁环境测试报告" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh_mm_ss") + ".docx"));
        document->dynamicCall("Close(boolean)", false);
        word.dynamicCall("Quit()");
    });
    mainLayout->addLayout(horizontalLayout);
}

bool WBSignalDetectWidget::GenerateExcelResistivityTable(const CommonInfoSet& CommonInfo, const ResistivitySet& ResistivityInfo)
{
    QFileDialog dialog;
    auto folderName = dialog.getExistingDirectory(nullptr, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderName.isEmpty())
    {
        qDebug() << "File saving cancelled.";
        return false;
    }

    QXlsx::Document xlsx;
    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::CellRange range("B2:D2");
    xlsx.mergeCells(range, format);
    xlsx.write("B2", QString("测量日期：%1").arg(CommonInfo.Date.toString(" yyyy年 MM月 dd日")), format);

    range = QXlsx::CellRange("E2:F2");
    xlsx.mergeCells(range, format);
    xlsx.write("E2", QString("测试地点：%1").arg(CommonInfo.TestPosition), format);

    xlsx.write("B3", "环境条件", format);

    range = QXlsx::CellRange("C3:F3");
    xlsx.mergeCells(range, format);
    xlsx.write("C3", QString("天气状况：%1 温度：%2℃ 湿度：%3%rh").arg(CommonInfo.Weather).arg(CommonInfo.Temprature).arg(CommonInfo.Humidity), format);

    xlsx.write("B4", "测量仪器", format);
    range = QXlsx::CellRange("C4:F4");
    xlsx.mergeCells(range, format);
    xlsx.write("C4", "短波接收天线 短波测量仪", format);

    xlsx.write("B5", "", format);
    for (auto i = 0; i < ResistivitySet::DIRECTIONS; ++i)
    {
        xlsx.write(QChar('C' + i) + QString("5"), QString("方向%1").arg(i + 1), format);
    }

    xlsx.write("B6", "测量值R(Ω)", format);
    xlsx.write("B7", "电阻率ρ(Ω•m)", format);
    xlsx.write("B8", "平均值ρ ̅(Ω•m)", format);
    xlsx.write("B9", "测量人员", format);

    for (auto i = 0; i < ResistivitySet::DIRECTIONS; ++i)
    {
        xlsx.write(QChar('C' + i) + QString("6"), QString::number(ResistivityInfo.Resistance[i]), format);
        xlsx.write(QChar('C' + i) + QString("7"), QString::number(ResistivityInfo.Resistivity[i]), format);
    }

    range = QXlsx::CellRange("C8:F8");
    xlsx.mergeCells(range, format);
    range = QXlsx::CellRange("C9:F9");
    xlsx.mergeCells(range, format);

    return xlsx.saveAs(folderName + "/土壤电阻率测量数据处理记录" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh_mm_ss") + ".xlsx");
}

bool WBSignalDetectWidget::GenerateExcelConductivityTable(const CommonInfoSet& CommonInfo, const ConductivitySet& ConductivityInfo)
{
    QFileDialog dialog;
    auto folderName = dialog.getExistingDirectory(nullptr, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (folderName.isEmpty())
    {
        qDebug() << "File saving cancelled.";
        return false;
    }

    QXlsx::Document xlsx;
    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::CellRange range("B2:D2");
    xlsx.mergeCells(range, format);
    xlsx.write("B2", QString("测量日期：%1").arg(CommonInfo.Date.toString(" yyyy年 MM月 dd日")), format);

    range = QXlsx::CellRange("E2:G2");
    xlsx.mergeCells(range, format);
    xlsx.write("E2", QString("测试地点：%1").arg(CommonInfo.TestPosition), format);

    xlsx.write("B3", "环境条件", format);

    range = QXlsx::CellRange("C3:G3");
    xlsx.mergeCells(range, format);
    xlsx.write("C3", QString("天气状况：%1 温度：%2℃ 湿度：%3%rh").arg(CommonInfo.Weather).arg(CommonInfo.Temprature).arg(CommonInfo.Humidity), format);

    xlsx.write("B4", "测量仪器", format);
    range = QXlsx::CellRange("C4:G4");
    xlsx.mergeCells(range, format);
    xlsx.write("C4", "短波接收天线 短波测量仪", format);

    xlsx.write("B5", "", format);
    for (auto i = 0; i < ConductivitySet::MEASURE_POSITION; ++i)
    {
        xlsx.write(QChar('C' + i) + QString("5"), QString("测量点%1").arg(i + 1), format);
    }

    xlsx.write("B6", "测量值R(Ω)", format);
    xlsx.write("B7", "均匀性", format);
    xlsx.write("B8", "测量人员", format);
    for (auto i = 0; i < ConductivitySet::MEASURE_POSITION; ++i)
    {
        xlsx.write(QChar('C' + i) + QString("6"), QString::number(ConductivityInfo.SoilResistance[i]), format);
    }

    range = QXlsx::CellRange("C7:G7");
    xlsx.mergeCells(range, format);
    range = QXlsx::CellRange("C8:G8");
    xlsx.mergeCells(range, format);
    return xlsx.saveAs(folderName + "/土壤导电均匀性测量数据记录" + QDateTime::currentDateTime().toString(" yyyy-MM-dd hh_mm_ss") + ".xlsx");
}
