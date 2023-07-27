#include "TableNoise.h"

#include <QHeaderView>

TableNoise::TableNoise(QWidget* parent): QTableView(parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setSortingEnabled(true);

    setModel(m_model = new QStandardItemModel);
    RenewItems();
}

void TableNoise::RenewItems()
{
    m_model->clear();
    m_model->setColumnCount(COLUMNS);

    m_model->setHeaderData(0, Qt::Horizontal, tr("典型频率点(MHz)"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("测量频率(MHz)"));
    m_model->setHeaderData(2, Qt::Horizontal, tr("时间(h:m)"));
    m_model->setHeaderData(3, Qt::Horizontal, tr("测量电平(dB/uV)"));
    m_model->setHeaderData(4, Qt::Horizontal, tr("平均电平(dB/uV)"));
    m_model->setHeaderData(5, Qt::Horizontal, tr("最大电平(dB/uV)"));
    m_model->setHeaderData(6, Qt::Horizontal, tr("最小电平(dB/uV)"));
    m_model->setHeaderData(7, Qt::Horizontal, tr("检波方式(dB/uV)"));
    m_model->setHeaderData(8, Qt::Horizontal, tr("中频带宽(dB/uV)"));
}
