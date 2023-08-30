#ifndef CHARTWIDGETWB_H
#define CHARTWIDGETWB_H

#include "ChartWidgetCombine.h"

class ChartWidgetWB: public ChartWidgetCombine
{
    Q_OBJECT
public:
    ChartWidgetWB(QString, QWidget* = nullptr);
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
    QCheckBox *MaxKeepSelect, *MinKeepSelect, *SpectrumSelect;

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

private slots:
    void SeriesSelectChanged();
};

#endif // CHARTWIDGETWB_H
