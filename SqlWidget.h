#ifndef SQLWIDGET_H
#define SQLWIDGET_H

#include <QWidget>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QTableView>
#include <QLabel>

#include "SqlData.h"

class SqlWidget: public QWidget
{
    Q_OBJECT
public:
    SqlWidget(QWidget* parent = nullptr);
    ~SqlWidget();

private:
    QDoubleSpinBox* startFreq;
    QDoubleSpinBox* endFreq;
    QDateTimeEdit* startTime;
    QDateTimeEdit* endTime;
    QSpinBox* confidenceGate;
    QTableView* tableView;
    SqlData sqlData;
    QSpinBox* pageNow;
    QLabel* pageTotal;
    static constexpr int PAGE_COUNT = 200;
};

#endif // SQLWIDGET_H
