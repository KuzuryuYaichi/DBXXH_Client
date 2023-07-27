#include "PointTableWidget.h"
#include <QApplication>

void PointTableView::InitialMenuCtrl()
{
    setSelectionMode(SingleSelection);
    setSelectionBehavior(QTableWidget::SelectRows);
    setContextMenuPolicy(Qt::CustomContextMenu);
    m_tabMenu = new QMenu(this);
    for (int i = 0; i < 3; ++i)
    {
        auto action = new QAction(tr("Add To Marker%1").arg(i + 1), this);
        m_tabMenu->addAction(action);
        connect(action, &QAction::triggered, this, [this] {
            auto indexes = selectionModel()->selectedIndexes();
            if (indexes.size() == 0)
                return;
            bool res;
            auto freq = indexes[0].data().toDouble(&res);
        });
    }
    connect(this, &QTableWidget::customContextMenuRequested, this, [this] (QPoint) {
        m_tabMenu->exec(QCursor::pos());
    });
}

void PointTableView::UpdateItems(const QVector<std::tuple<double, double, int>>& data, const QDateTime& time)
{
    if (!m_continueFlag)
        return;
    auto length = data.size();
    if (data.size() == 0)
        return;
    RenewItems();
    auto timeStr = time.toString("yyyy-MM-dd hh:mm:ss");
    for (auto i = 0; i < length; ++i)
    {
        auto& d = data[i];
        InsertRows(QList<QStandardItem*>{ new QStandardItem(QString::number(std::get<0>(d))),
                                          new QStandardItem(QString::number(std::get<1>(d))),
                                          new QStandardItem(QString::number(std::get<2>(d))),
                                          new QStandardItem(timeStr)});
    }
}

void PointTableView::RenewItems()
{
    m_model->clear();
    m_model->setColumnCount(COLUMNS);
    m_model->setHeaderData(0, Qt::Horizontal, tr("Freq(MHz)"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("Direction(degree)"));
    m_model->setHeaderData(3, Qt::Horizontal, tr("Confidence Level(%)"));
    m_model->setHeaderData(2, Qt::Horizontal, tr("Time"));
}

void PointTableView::InsertRows(QList<QStandardItem*> items)
{
    m_model->appendRow(items);
}

PointTableView::PointTableView(QPushButton* continueBtn, QWidget *parent): QTableView(parent), continueBtn(continueBtn)
{
    InitialMenuCtrl();
    installEventFilter(this);

    setModel(m_model = new QStandardItemModel);
    RenewItems();

    connect(continueBtn, &QPushButton::clicked, this, [this] (bool) {
        this->continueBtn->setIcon(QApplication::style()->standardIcon((m_continueFlag = !m_continueFlag)? QStyle::SP_MediaPause: QStyle::SP_MediaPlay));
    });
}

