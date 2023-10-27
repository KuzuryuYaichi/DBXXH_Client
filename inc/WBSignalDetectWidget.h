#ifndef WBSIGNALDETECTWIDGET_H
#define WBSIGNALDETECTWIDGET_H

#include <QWidget>

#include "../inc/SignalDetectTableView.h"
#include "../inc/DisturbNoiseTableView.h"
#include "../inc/ManMadeNoiseTableView.h"
#include "../inc/SignalNoiseModel.h"
#include "PulseDetectTableView.h"
#include "../inc/PopupParamDialog.h"
#include "../inc/TypicalFreqDialog.h"
#include "CommonInfoDialog.h"
#include "ResistivityDialog.h"
#include "ConductivityDialog.h"

class WBSignalDetectWidget: public QWidget
{
    Q_OBJECT
public:
    explicit WBSignalDetectWidget(QWidget* = nullptr);
    void sigTriggerSignalDetect(unsigned char*, int, int, int);
    void PulseDetect(Pulse*, int);

protected:
    bool GenerateExcelResistivityTable(const CommonInfoSet&, const ResistivitySet&);
    bool GenerateExcelConductivityTable(const CommonInfoSet&, const ConductivitySet&);

//signals:
//    //记录开始检测时间
//    void startDetect();
//    //记录最终完成检测时间
//    void stopDetect();

private:
    void setupUi();

    SignalNoiseModel* m_pSignalDetectModel;
    SignalDetectTableView* m_pSignalDetectTable;
    DisturbNoiseTableView* m_pDisturbNoiseTable;
    ManMadeNoiseTableView* m_pManMadeNoiseTable;
    PulseDetectTableView* m_pPulseDetectTable;
    PopupParamDialog* m_pPopupParamDialog;
    TypicalFreqDialog* m_pTypicalFreqDialog;
    ParamSet m_DetectParam;
    CommonInfoDialog *m_CommonInfoDialog;
    CommonInfoSet m_CommonInfo;
    ResistivityDialog *m_ResistivityDialog;
    ConductivityDialog *m_ConductivityDialog;

    QTabWidget *tabWidget_SignalDetectTable;
    QPushButton *pushButton_ParamSet;
    QPushButton *pushButton_TypicalFreqSet;
    QPushButton *pushButton_importLegal;
    QPushButton *pushButton_ExportLegal;
    QPushButton *pushButton_cleanAllData;
    QPushButton *pushButton_setLegalFreq;
    QPushButton *pushButton_CommonInfo;
    QPushButton *pushButton_GenerateSignalDetect;
    QPushButton *pushButton_GenerateDisturbSignal;
    QPushButton *pushButton_GenerateManMadeNoise;
    QPushButton *pushButton_GenerateResistivity;
    QPushButton *pushButton_GenerateConductivity;
    QPushButton *pushButton_GenerateElecEnvReport;

    static constexpr char REPORT_NAME[] = "电磁环境测试报告";
};

#endif // WBSIGNALDETECTWIDGET_H
