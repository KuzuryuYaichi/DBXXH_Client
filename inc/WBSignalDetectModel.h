#ifndef WBSIGNALDETECTMODEL_H
#define WBSIGNALDETECTMODEL_H

#include <QAbstractTableModel>
#include <QAxObject>
#include <QAxWidget>
#include <QUuid>
#include <mutex>

#include "inc/CommonInfoDialog.h"
#include "ResistivityDialog.h"
#include "ConductivityDialog.h"

#include "ipp.h"
#include "ippcore.h"
#include "ippvm.h"
#include "ipps.h"

class WBSignalDetectModel: public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit WBSignalDetectModel(QObject* = nullptr);
    int rowCount(const QModelIndex& = QModelIndex()) const override;
    bool setData(const QModelIndex&, const QVariant&, int = Qt::EditRole) override;
    QVariant data(const QModelIndex&, int = Qt::DisplayRole) const override;

    bool bIsDetecting() const;

public slots:
    //更新表格中数据，用户在选择当前信号是否合法时不进行界面更新
    virtual void UpdateData() = 0;
//    void SetStartTime();
//    void SetStopTime();

protected:
    void TimeRecord();
    void MergeCells(QAxObject*, int, int, int, int);
    std::vector<std::pair<QUuid, std::vector<QVariant>>> m_DisplayData;
    qint64 m_i64SystemStartTime = 0;                //系统启动时的时间，软件启动时开始计算，用于计算信号占用率
    qint64 m_i64SystemStopTime = 0;                 //系统/信号分析 停止时的时间，用于计算信号占用率，也作为当前处理的停止时间
    qint64 m_i64CurrentDetectStartTime = 0;         //当前信号处理的开始时间
    std::mutex m_mutex;

private:
    QTimer* m_pSignalActiveChecker;
    bool m_bIsDetecting = true /*false*/;            //用于记录当前是否正在检测信号
};

#endif // WBSIGNALDETECTMODEL_H
