#ifndef CHARTWIDGETCOMBINE_H
#define CHARTWIDGETCOMBINE_H

#include "ChartViewWaterfall.h"

class ChartWidgetCombine: public QWidget
{
    Q_OBJECT
public:
    ChartWidgetCombine(QWidget* = nullptr);
    virtual void replace(const std::shared_ptr<unsigned char[]>&) = 0;
    virtual void ChangeMode(int) = 0;
    ChartViewWaterfall* chartWaterfall;

protected:
    QComboBox* showBox;
    QDoubleSpinBox* freqEdit;
    QComboBox* boundBox;
    QVBoxLayout* mainLayout;
    QHBoxLayout* layoutSpectrum;
    QHBoxLayout* hBoxLayout;
    bool ready = true;
    QTimer* m_updater;

private:
    static constexpr int REFRESH_INTERVAL = 150;
};

#endif // CHARTWIDGETCOMBINE_H
