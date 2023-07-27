#ifndef TABLEINTERFERENCE_H
#define TABLEINTERFERENCE_H

#include <QTableView>
#include <QStandardItemModel>

class TableInterference: public QTableView
{
    Q_OBJECT
public:
    TableInterference(QWidget* = nullptr);
    void RenewItems();

private:
    QStandardItemModel* m_model = nullptr;
    static constexpr int COLUMNS = 3;
};

#endif // TABLEINTERFERENCE_H
