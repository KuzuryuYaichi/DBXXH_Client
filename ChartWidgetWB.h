#ifndef CHARTWIDGETWB_H
#define CHARTWIDGETWB_H

#include "ChartWidgetCombine.h"
#include "ChartViewSpectrumWB.h"
#include "ChartViewHeatmap.h"
#include "ChartViewAfterglow.h"

class ChartWidgetWB: public ChartWidgetCombine
{
    Q_OBJECT
public:
    ChartWidgetWB(QString, QWidget* = nullptr);
    virtual void ChangeMode(int) override;
    virtual void replace(const std::shared_ptr<unsigned char[]>&) override;
    ChartViewSpectrumWB* chartSpectrum;
    ChartViewHeatmap* chartHeatmap;
    ChartViewAfterglow* chartAfterglow;

signals:
    void ParamsChanged();

protected:
    enum SHOW_MODE
    {
        SPECTRUM_MODE = 0,
        HEATMAP_MODE,
        AFTERFLOW_MODE
    };

private:
    QButtonGroup* RfGainModeGroup;
    QDoubleSpinBox *RfGainEdit;
    QDoubleSpinBox *DigitGainEdit;
    QButtonGroup *FeedbackGroup;
    QComboBox *freqResBox;
    QComboBox *smoothBox;
    QCheckBox *MaxKeepSelect, *MinKeepSelect;

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
