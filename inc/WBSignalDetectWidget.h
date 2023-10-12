#ifndef WBSIGNALDETECTWIDGET_H
#define WBSIGNALDETECTWIDGET_H

#include <QWidget>

#include "../inc/PopupParamSet.h"
#include "../inc/TypicalFreqSetWidget.h"
#include "../inc/SignalDetectTableView.h"
#include "../inc/ManMadeNoiseTableView.h"
#include "../inc/DisturbNoiseTableView.h"
#include "../inc/WBSignalDetectModel.h"
#include "PulseDetectTableView.h"
#include "PulseDetectModel.h"

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

signals:
    //记录开始检测时间
    void startDetect();
    //记录最终完成检测时间
    void stopDetect();

public slots:
    void GenerateWord();

protected:
    void GenerateSignalDetectTable(WBSignalDetectModel*);
    void GenerateManMadeNoiseTable(WBSignalDetectModel*);

private:
    void setupUi();

    WBSignalDetectModel* m_pSignalDetectModel;
    WBSignalDetectModel* m_pManMadeNoiseModel;
    PulseDetectModel* m_pPulseDetectModel;
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
    QPushButton *pushButton_GenerateDisturbSIgnal;
    QPushButton *pushButton_GenerateManMadeNoise;
    QPushButton *pushButton_GenerateElecEnvReport;
};

#endif // WBSIGNALDETECTWIDGET_H
