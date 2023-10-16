#ifndef PULSEDETECTTABLEVIEW_H
#define PULSEDETECTTABLEVIEW_H

#include <QTableView>

#include "PulseDetectModel.h"

class PulseDetectTableView: public QTableView
{
    Q_OBJECT
public:
    explicit PulseDetectTableView(QWidget* = nullptr);
    PulseDetectModel* m_pPulseDetectModel;
};

#endif // PULSEDETECTTABLEVIEW_H
