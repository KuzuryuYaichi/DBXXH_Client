#include "inc/DisturbNoiseTableView.h"

#include <QHeaderView>
#include <QMessageBox>

DisturbNoiseTableView::DisturbNoiseTableView(QWidget *parent): QTableView (parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    verticalHeader()->hide();
}
