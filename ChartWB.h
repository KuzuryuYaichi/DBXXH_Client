#ifndef CHARTWB_H
#define CHARTWB_H

#include "CombineWidget.h"

class ChartWB: public CombineWidget
{
    Q_OBJECT
public:
    ChartWB(QString, QWidget* = nullptr);
signals:
    void ParamsChanged();

private:
    QButtonGroup* RfGainModeGroup;
    QDoubleSpinBox *RfGainEdit;
    QDoubleSpinBox *DigitGainEdit;
    QLineEdit *LmValEdit;
    QButtonGroup *FeedbackGroup;
    QComboBox *freqResBox;
    QComboBox *smoothBox;

    static constexpr double RESOLUTIONS[] =
    {
        0.092,
        0.183,
        0.366,
        0.732,
        1.465,
        2.930,
        5.859,
        11.719,
        23.438,
        46.875
    };
};

#endif // CHARTWB_H
