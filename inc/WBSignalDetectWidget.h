#ifndef WBSIGNALDETECTWIDGET_H
#define WBSIGNALDETECTWIDGET_H

#include <QWidget>

#include "../inc/PopupParamSet.h"
#include "../inc/TypicalFreqSetWidget.h"
#include "../inc/SignalDetectTableView.h"
#include "../inc/DisturbNoiseTableView.h"
#include "../inc/ManMadeNoiseTableView.h"
#include "../inc/SignalNoiseModel.h"
#include "PulseDetectTableView.h"

class WBSignalDetectWidget: public QWidget
{
    Q_OBJECT
public:
    explicit WBSignalDetectWidget(QWidget* = nullptr);
    //单次数据传入触发处理
    void sigTriggerSignalDetect(unsigned char*, int, int, int); //当前FFT覆盖带宽
    void PulseDetect(Pulse*, int);
public slots:
    void sigSetValidAmpThreshold(float); //设置有效电平门限 dBm

//signals:
//    //记录开始检测时间
//    void startDetect();
//    //记录最终完成检测时间
//    void stopDetect();

protected:
    void GenerateSignalDetectTable(WBSignalDetectModel*);
    void GenerateManMadeNoiseTable(WBSignalDetectModel*);

private:
    void setupUi();

    SignalNoiseModel* m_pSignalDetectModel;
    SignalDetectTableView* m_pSignalDetectTable;
    DisturbNoiseTableView* m_pDisturbNoiseTable;
    ManMadeNoiseTableView* m_pManMadeNoiseTable;
    PulseDetectTableView* m_pPulseDetectTable;
    PopupParamSet* m_pPopupParamSet;
    TypicalFreqSetWidget* m_pTypicalFreqSetWidget;
    ParamSet m_DetectParam;

    QPushButton *pushButton_ParamSet;
    QPushButton *pushButton_TypicalFreqSet;
    QPushButton *pushButton_importLegal;
    QPushButton *pushButton_ExportLegal;
    QPushButton *pushButton_cleanAllData;
    QPushButton *pushButton_setLegalFreq;
    QTabWidget *tabWidget_SignalDetectTable;
    QPushButton *pushButton_GenerateSignalDetect;
    QPushButton *pushButton_GenerateDisturbSignal;
    QPushButton *pushButton_GenerateManMadeNoise;
    QPushButton *pushButton_GenerateElecEnvReport;
};

#endif // WBSIGNALDETECTWIDGET_H
