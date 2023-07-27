#ifndef TABLENOISE_H
#define TABLENOISE_H

#include <QTableView>
#include <QStandardItemModel>

class TableNoise: public QTableView
{
    Q_OBJECT
public:
    TableNoise(QWidget* = nullptr);
    void RenewItems();

private:
    QStandardItemModel* m_model = nullptr;
    static constexpr int COLUMNS = 7;
};

#endif // TABLENOISE_H
