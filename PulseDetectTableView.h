#ifndef PULSEDETECTTABLEVIEW_H
#define PULSEDETECTTABLEVIEW_H

#include <QTableView>

class PulseDetectTableView: public QTableView
{
    Q_OBJECT
public:
    explicit PulseDetectTableView(QWidget* = nullptr);
};

#endif // PULSEDETECTTABLEVIEW_H
