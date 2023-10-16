#include "inc/WBSignalDetectModel.h"
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

int WBSignalDetectModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid()? 0: (int)m_DisplayData.size();
}

bool WBSignalDetectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        m_DisplayData[index.row()][index.column()] = value;
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QVariant WBSignalDetectModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole) //显示内容
            return m_DisplayData[index.row()][index.column()];
        else if (role == Qt::TextAlignmentRole) //内容排版
            return Qt::AlignCenter;
    }
    return QVariant();
}

bool WBSignalDetectModel::bIsDetecting() const
{
    return m_bIsDetecting;
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

//void WBSignalDetectModel::SetStartTime()
//{
//    m_i64CurrentDetectStartTime = QDateTime::currentMSecsSinceEpoch();
//    m_bIsDetecting = true;
//}

//void WBSignalDetectModel::SetStopTime()
//{
//    m_bIsDetecting = false;
//    m_i64SystemStopTime = QDateTime::currentMSecsSinceEpoch();
//}
