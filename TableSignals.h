#ifndef TABLESIGNALS_H
#define TABLESIGNALS_H

#include <QTableView>
#include <QStandardItemModel>

class TableSignals: public QTableView
{
    Q_OBJECT
public:
    TableSignals(QWidget* = nullptr);
    void RenewItems();

private:
    QStandardItemModel* m_model = nullptr;
    static constexpr int COLUMNS = 7;
};

#endif // TABLESIGNALS_H
