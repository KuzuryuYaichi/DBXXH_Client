#ifndef WBSIGNALDETECTWIDGET_H
#define WBSIGNALDETECTWIDGET_H

#include <QWidget>

#include "SignalDetect/SignalDetectTableView.h"
#include "SignalDetect/DisturbNoiseTableView.h"
#include "SignalDetect/ManMadeNoiseTableView.h"
#include "SignalDetect/SignalNoiseModel.h"
#include "PulseDetectTableView.h"
#include "SignalDetect/PopupParamDialog.h"
#include "SignalDetect/TypicalFreqDialog.h"
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

signals:
    void DetectSwitch(bool);

private:
    void setupUi();

    SignalNoiseModel* m_pSignalDetectModel;
    SignalDetectTableView* m_pSignalDetectTable;
    DisturbNoiseTableView* m_pDisturbNoiseTable;
    ManMadeNoiseTableView* m_pManMadeNoiseTable;
    PulseDetectTableView* m_pPulseDetectTable;
    PopupParamDialog* m_pPopupParamDialog;
    TypicalFreqDialog* m_pTypicalFreqDialog;
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
    QPushButton *pushButton_setRemark;
    QPushButton *pushButton_CommonInfo;
    QPushButton *pushButton_GenerateSignalDetect;
    QPushButton *pushButton_GenerateDisturbSignal;
    QPushButton *pushButton_GenerateManMadeNoise;
    QPushButton *pushButton_GenerateResistivity;
    QPushButton *pushButton_GenerateConductivity;
    QPushButton *pushButton_GenerateElecEnvReport;
    QPushButton *pushButton_Detect;

    static constexpr char REPORT_NAME[] = "电磁环境测试报告";
};

#endif // WBSIGNALDETECTWIDGET_H
