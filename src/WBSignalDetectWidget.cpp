#include "../inc/WBSignalDetectWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QAxWidget>
#include <QAxObject>
#include "global.h"

WBSignalDetectWidget::WBSignalDetectWidget(QWidget *parent): QWidget(parent)
{
    setupUi();

    connect(this, &WBSignalDetectWidget::startDetect, m_pSignalDetectModel, &WBSignalDetectModel::SetStartTime);
    connect(this, &WBSignalDetectWidget::stopDetect, m_pSignalDetectModel, &WBSignalDetectModel::SetStopTime);
    connect(this, &WBSignalDetectWidget::startDetect, m_pManMadeNoiseModel, &WBSignalDetectModel::SetStartTime);
    connect(this, &WBSignalDetectWidget::stopDetect, m_pManMadeNoiseModel, &WBSignalDetectModel::SetStopTime);

    connect(m_pPopupParamSet = new PopupParamSet, &PopupParamSet::sigUpdateParam, this, [this](ParamSet param) {
        m_DetectParam = param;
        m_pSignalDetectModel->setBandwidthThreshold(m_DetectParam.BandwidthThreshold);
        m_pSignalDetectModel->setActiveThreshold(m_DetectParam.ActiveThreshold);
        m_pSignalDetectModel->setFreqPointThreshold(m_DetectParam.FreqPointThreshold);
        m_pSignalDetectModel->setAmplThreshold(m_DetectParam.AmplThreshold);
    });
    m_pPopupParamSet->setModal(false);
    m_pPopupParamSet->hide();
    connect(m_pTypicalFreqSetWidget = new TypicalFreqSetWidget, &TypicalFreqSetWidget::sigHaveTypicalFreq, m_pManMadeNoiseModel, &WBSignalDetectModel::setMapTypicalFreqAndItsTestFreq);
    m_pTypicalFreqSetWidget->setModal(false);
    m_pTypicalFreqSetWidget->hide();
}

void WBSignalDetectWidget::sigTriggerSignalDetect(unsigned char* amplData, int length, int StartFreq, int BandWidth)
{
    auto FFtin = ippsMalloc_32f(length);
    for (int i = 0; i < length; ++i)
    {
        FFtin[i] = (short)amplData[i] + AMPL_OFFSET;
    }
    m_pSignalDetectModel->findPeakCyclically(FFtin, length, StartFreq, BandWidth);
    m_pManMadeNoiseModel->findNoiseCharaAroundTypicalFreq(FFtin, length, StartFreq, BandWidth);
    ippsFree(FFtin);
}

void WBSignalDetectWidget::sigSetValidAmpThreshold(float amp)
{
    m_pSignalDetectModel->setAmplThreshold(amp);
}

void WBSignalDetectWidget::PulseDetect(Pulse* pulse, int len)
{
    m_pPulseDetectModel->replace(pulse, len);
}

void WBSignalDetectWidget::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    auto horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(pushButton_ParamSet = new QPushButton("参数设置"));
    connect(pushButton_ParamSet, &QPushButton::clicked, this, [this] {
        m_pPopupParamSet->setModal(true);
        m_pPopupParamSet->show();
    });
    horizontalLayout->addWidget(pushButton_TypicalFreqSet = new QPushButton("典型频点设置"));
    connect(this, &WBSignalDetectWidget::startDetect, this, [this] {
        pushButton_TypicalFreqSet->setVisible(false);
    });
    connect(this, &WBSignalDetectWidget::stopDetect, this, [this] {
        pushButton_TypicalFreqSet->setVisible(true);
    });
    connect(pushButton_TypicalFreqSet, &QPushButton::clicked, this, [this] {
        m_pTypicalFreqSetWidget->setModal(true);
        m_pTypicalFreqSetWidget->SetCurrentTypicalFreqFromTable(m_pManMadeNoiseModel->lstTypicalFreq());
        m_pTypicalFreqSetWidget->show();
    });
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_importLegal = new QPushButton("导入非法频点设置"));
    connect(pushButton_importLegal, &QPushButton::clicked, this, [this] {
        QMessageBox::information(nullptr, "导入非法频点设置", (m_pSignalDetectModel && m_pSignalDetectModel->SlotImportLegalFreqConf())? "导入成功！": "导入失败！");
    });
    horizontalLayout->addWidget(pushButton_ExportLegal = new QPushButton("导出非法频点设置"));
    connect(pushButton_ExportLegal, &QPushButton::clicked, this, [this] {
        QMessageBox::information(nullptr, "导出非法频点设置", (m_pSignalDetectModel && m_pSignalDetectModel->SlotExportLegalFreqConf())? "导出成功！": "导出失败！");
    });
    horizontalLayout->addWidget(pushButton_cleanAllData = new QPushButton("清理"));
    connect(pushButton_cleanAllData, &QPushButton::clicked, this, [this] {
        m_pSignalDetectModel->SlotCleanUp();
    });
    horizontalLayout->addWidget(pushButton_setLegalFreq = new QPushButton("开始设置非法频点"));
    connect(pushButton_setLegalFreq, &QPushButton::clicked, this, [this](bool checked) {
        pushButton_setLegalFreq->setText(checked? "完成设置": "开始设置非法频点");
        m_pSignalDetectModel->SlotTriggerLegalFreqSet(checked);
    });
    pushButton_setLegalFreq->setCheckable(true);

    mainLayout->addLayout(horizontalLayout);
    mainLayout->addWidget(tabWidget_SignalDetectTable = new QTabWidget);
    tabWidget_SignalDetectTable->addTab(m_pSignalDetectTable = new SignalDetectTableView, "信号检测表");
    m_pSignalDetectTable->setModel(m_pSignalDetectModel = new WBSignalDetectModel);
    tabWidget_SignalDetectTable->addTab(m_pDisturbNoiseTable = new DisturbNoiseTableView, "干扰信号测量表");
    m_pDisturbNoiseTable->setModel(m_pSignalDetectModel);
    tabWidget_SignalDetectTable->addTab(m_pManMadeNoiseTable = new ManMadeNoiseTableView, "电磁环境人为噪声电平测量表");
    m_pManMadeNoiseTable->setModel(m_pManMadeNoiseModel = new WBSignalDetectModel);
    tabWidget_SignalDetectTable->addTab(m_pPulseDetectTable = new PulseDetectTableView, "脉冲检测表");
    m_pPulseDetectTable->setModel(m_pPulseDetectModel = new PulseDetectModel);
    connect(tabWidget_SignalDetectTable, &QTabWidget::currentChanged, this, [this] (int) {
        emit pushButton_setLegalFreq->clicked(false);
        pushButton_setLegalFreq->setVisible(!tabWidget_SignalDetectTable->currentIndex());
        switch (tabWidget_SignalDetectTable->currentIndex())
        {
        case 0: m_pSignalDetectModel->setUserViewType(SIGNAL_DETECT_TABLE); break;
        case 1: m_pSignalDetectModel->setUserViewType(DISTURB_NOISE_TABLE); break;
        case 2: m_pManMadeNoiseModel->setUserViewType(MAN_MADE_NOISE_TABLE); break;
        }
        emit m_pSignalDetectModel->sigTriggerRefreshData();
    });

    horizontalLayout = new QHBoxLayout;
    horizontalLayout->addStretch();
    horizontalLayout->addWidget(pushButton_GenerateSignalDetect = new QPushButton("保存信号检测记录"));
    connect(pushButton_GenerateSignalDetect, &QPushButton::clicked, this, [this]
    {
        tabWidget_SignalDetectTable->setCurrentIndex(0); //先将tabwidget转到对应的tab上
        QFileDialog dialog;
        auto selectedFolder = dialog.getExistingDirectory(this, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (selectedFolder.isEmpty())
        {
            qDebug() << "File saving cancelled.";
            return;
        }
        if (!m_pSignalDetectTable->GenerateExcelTable(selectedFolder))
        {
            //TODO: 生成失败时的处理方法
        }
    });
    horizontalLayout->addWidget(pushButton_GenerateDisturbSIgnal = new QPushButton("保存干扰信号测量记录"));
    connect(pushButton_GenerateDisturbSIgnal, &QPushButton::clicked, this, [this]
    {
        tabWidget_SignalDetectTable->setCurrentIndex(1); //先将tabwidget转到对应的tab上
        QFileDialog dialog;
        QString selectedFolder = dialog.getExistingDirectory(this, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (selectedFolder.isEmpty())
        {
            qDebug() << "File saving cancelled.";
            return;
        }
        if (!m_pDisturbNoiseTable->GenerateExcelTable(selectedFolder))
        {
            //TODO: 生成失败时的处理方法
        }
    });
    horizontalLayout->addWidget(pushButton_GenerateManMadeNoise = new QPushButton("保存电磁环境人为噪声电平测量记录"));
    connect(pushButton_GenerateManMadeNoise, &QPushButton::clicked, this, [this]
    {
        tabWidget_SignalDetectTable->setCurrentIndex(2); //先将tabwidget转到对应的tab上
        QFileDialog dialog;
        QString selectedFolder = dialog.getExistingDirectory(this, tr("Select Directory"), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (selectedFolder.isEmpty())
        {
            qDebug() << "File saving cancelled.";
            return;
        }
        if (!m_pManMadeNoiseTable->GenerateExcelTable(selectedFolder, m_pManMadeNoiseModel->mapExistTypicalFreqNoiseRecordAmount()))
        {
            //TODO: 生成失败时的处理方法
        }
    });
    horizontalLayout->addWidget(pushButton_GenerateElecEnvReport = new QPushButton("生成电磁环境测试报告"));
    connect(pushButton_GenerateElecEnvReport, &QPushButton::clicked, this, &WBSignalDetectWidget::GenerateWord);
    mainLayout->addLayout(horizontalLayout);
}

void WBSignalDetectWidget::GenerateWord()
{
    QAxWidget word("Word.Application", 0, Qt::MSWindowsOwnDC);
    word.setProperty("Visible", false);
    auto documents = word.querySubObject("Documents");
    documents->dynamicCall("Add(QString)", QDir::currentPath() + "/ReportFormat.docx");
    auto document = word.querySubObject("ActiveDocument");
    if (!document || document->isNull())
        return;
    m_pManMadeNoiseTable->GenerateManMadeNoiseTable(document, m_pManMadeNoiseModel->mapExistTypicalFreqNoiseRecordAmount());
    m_pSignalDetectTable->GenerateSignalDetectTable(document);
    QString pathsave = QFileDialog::getSaveFileName(this, "Save", "../", "word(*doc)");
    if(pathsave.isEmpty())
        return;
    document->dynamicCall("SaveAs(const QString&))", QDir::toNativeSeparators(pathsave));
    document->dynamicCall("Close(boolean)", false);
    word.dynamicCall("Quit()");
    QMessageBox::warning(this, "完成", "文件已经保存", QMessageBox::Yes);
}

void OutputReport()
{
    auto filepath = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save Report"), "/untitled.docx", QObject::tr("Microsoft Word 2007 (*.docx)"));
    if (filepath.isNull())
        return;
    QAxObject word("Word.Application");
    word.setProperty("Visible", false);
    auto documents = word.querySubObject("Documents"); //获取所有工作文档
    QString sTemp = QDir::currentPath() + QString("ReportTemplate.dotx"); //创建一个word文档
    documents->dynamicCall("Add(QString)", sTemp);

    auto document = word.querySubObject("ActiveDocument");
    auto bookmark_table = document->querySubObject("Bookmarks(QVariant)", "dataTable")->querySubObject("Range"); //找到书签位置
    auto tables = document->querySubObject("Tables"); //添加表格
    QVariantList params;
    params.append(bookmark_table->asVariant());
    params.append(3);  //行数
    params.append(4);  //列数

    //表格对象
    auto datatable = tables->querySubObject("Add(QAxObject*, int, int, QVariant&, QVariant&)", params);
    datatable->setProperty("Style", "网格型");

    //设置表头
    auto rangeTitle = datatable->querySubObject("Cell(int, int)", 1, 1)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", QStringLiteral("序号"));

    rangeTitle = datatable->querySubObject("Cell(int, int)", 1, 2)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", QStringLiteral("测量值"));

    rangeTitle = datatable->querySubObject("Cell(int, int)", 1, 3)->querySubObject("Range");
    rangeTitle->querySubObject("ParagraphFormat")->setProperty("Alignment", "wdAlignParagraphCenter");
    rangeTitle->querySubObject("Font")->setProperty("Size", 10.5);
    rangeTitle->dynamicCall("SetText(QString)", QStringLiteral("理论值"));

    document->dynamicCall("SaveAs(const QString&)", QString(QDir::toNativeSeparators(filepath)));
    document->dynamicCall("Close (boolean)", true); //关闭文档

    word.dynamicCall("Quit (void)"); //退出
    QMessageBox::about(NULL, QStringLiteral("提示"), QStringLiteral("检测报告已生成!"));
}
