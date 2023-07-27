#include "TableSignals.h"

#include <QHeaderView>

TableSignals::TableSignals(QWidget* parent): QTableView(parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setSortingEnabled(true);

    setModel(m_model = new QStandardItemModel);
    RenewItems();
}

void TableSignals::RenewItems()
{
    m_model->clear();
    m_model->setColumnCount(COLUMNS);

    m_model->setHeaderData(0, Qt::Horizontal, tr("中心频率"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("电平"));
    m_model->setHeaderData(2, Qt::Horizontal, tr("带宽"));
    m_model->setHeaderData(3, Qt::Horizontal, tr("起始时间"));
    m_model->setHeaderData(4, Qt::Horizontal, tr("结束时间"));
    m_model->setHeaderData(5, Qt::Horizontal, tr("占用带宽"));
    m_model->setHeaderData(6, Qt::Horizontal, tr("信号占用度"));
}
