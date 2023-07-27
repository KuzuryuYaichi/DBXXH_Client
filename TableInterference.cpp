#include "TableInterference.h"

#include <QHeaderView>

TableInterference::TableInterference(QWidget* parent): QTableView(parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setSortingEnabled(true);

    setModel(m_model = new QStandardItemModel);
    RenewItems();
}

void TableInterference::RenewItems()
{
    m_model->clear();
    m_model->setColumnCount(COLUMNS);

    m_model->setHeaderData(0, Qt::Horizontal, tr("中心频率(MHz)"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("大信号电平(dBuV)"));
    m_model->setHeaderData(2, Qt::Horizontal, tr("测量时间(h:m)"));
}
