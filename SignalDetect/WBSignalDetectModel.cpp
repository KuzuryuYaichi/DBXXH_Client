#include "SignalDetect/WBSignalDetectModel.h"
#include <QDateTime>
#include <QTimer>
#include <QSettings>

WBSignalDetectModel::WBSignalDetectModel(QObject *parent): QAbstractTableModel(parent)
{
    m_pSignalActiveChecker = new QTimer(this);
    m_pSignalActiveChecker->setInterval(1000);
    connect(m_pSignalActiveChecker, &QTimer::timeout, this, &WBSignalDetectModel::UpdateData);
    m_pSignalActiveChecker->start();
}

void WBSignalDetectModel::TimeRecord()
{
    if (m_i64SystemStartTime == 0)
    {
        m_i64SystemStartTime = m_i64SystemStopTime = m_i64CurrentDetectStartTime = QDateTime::currentMSecsSinceEpoch();
    }
}

void WBSignalDetectModel::MergeCells(QAxObject* table, int nStartRow, int nStartCol, int nEndRow, int nEndCol)
{
    auto StartCell = table->querySubObject("Cell(int, int)", nStartRow, nStartCol);
    auto EndCell = table->querySubObject("Cell(int, int)", nEndRow, nEndCol);
    StartCell->querySubObject("Merge(QAxObject*)", EndCell->asVariant());
}

void WBSignalDetectModel::SetTime(bool isDetecting)
{
    if (isDetecting)
        m_i64CurrentDetectStartTime = QDateTime::currentMSecsSinceEpoch();
    else
        m_i64SystemStopTime = QDateTime::currentMSecsSinceEpoch();
    m_bIsDetecting = isDetecting;
}
