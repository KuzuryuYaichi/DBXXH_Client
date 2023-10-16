#include "inc/ManMadeNoiseTableView.h"
#include <QHeaderView>
#include <QMessageBox>

ManMadeNoiseTableView::ManMadeNoiseTableView(QWidget *parent): QTableView(parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSortingEnabled(true);
    verticalHeader()->hide();
    setModel(m_pManMadeNoiseModel = new ManMadeNoiseModel);
}
