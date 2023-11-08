#ifndef WBSIGNALDETECTMODEL_H
#define WBSIGNALDETECTMODEL_H

#include <QAbstractTableModel>
#include <QAxObject>
#include <QAxWidget>
#include <QUuid>
#include <mutex>

#include "ipp.h"
#include "ippcore.h"
#include "ippvm.h"
#include "ipps.h"

class WBSignalDetectModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit WBSignalDetectModel(QObject* = nullptr);
    bool m_bIsDetecting = false; //用于记录当前是否正在检测信号

public slots:
    virtual void UpdateData() = 0; //更新表格中数据，用户在选择当前信号是否合法时不进行界面更新
    virtual void SetTime(bool);

protected:
    void TimeRecord();
    void MergeCells(QAxObject*, int, int, int, int);
    qint64 m_i64SystemStartTime = 0;                //系统启动时的时间，软件启动时开始计算，用于计算信号占用率
    qint64 m_i64SystemStopTime = 0;                 //系统/信号分析 停止时的时间，用于计算信号占用率，也作为当前处理的停止时间
    qint64 m_i64CurrentDetectStartTime = 0;         //当前信号处理的开始时间
    std::mutex m_mutex;

private:
    QTimer* m_pSignalActiveChecker;
};

#endif // WBSIGNALDETECTMODEL_H
