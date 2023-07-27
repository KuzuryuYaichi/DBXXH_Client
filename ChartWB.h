#ifndef CHARTWB_H
#define CHARTWB_H

#include "CombineWidget.h"

class ChartWB: public CombineWidget
{
    Q_OBJECT
public:
    ChartWB(QString, QString, int, int, QString, int, int, QWidget* = nullptr);
};

#endif // CHARTWB_H
