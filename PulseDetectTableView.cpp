#include "PulseDetectTableView.h"

#include <QHeaderView>

PulseDetectTableView::PulseDetectTableView(QWidget *parent): QTableView(parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    verticalHeader()->hide();
}
