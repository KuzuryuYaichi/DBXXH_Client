#ifndef POINTTABLEWIDGET_H
#define POINTTABLEWIDGET_H

#include <QTableWidget>
#include <QMenu>
#include <QStandardItemModel>
#include <QPushButton>
#include <QDateTime>
#include <tuple>

class PointTableView: public QTableView
{
    Q_OBJECT
public:
    PointTableView(QPushButton*, QWidget* = nullptr);

signals:
    void SetFreqEdit(double);

public slots:
    void UpdateItems(const QVector<std::tuple<double, double, int>>&, const QDateTime&);

private:
    void InitialMenuCtrl();
    void RenewItems();
    void InsertRows(QList<QStandardItem*>);
    QMenu* m_tabMenu;
    QPushButton* continueBtn;
    QStandardItemModel* m_model = nullptr;
    bool m_continueFlag = true;

    static constexpr int COLUMNS = 4;
};

#endif // POINTTABLEWIDGET_H
