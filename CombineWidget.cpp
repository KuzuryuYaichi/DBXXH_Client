#include "CombineWidget.h"

CombineWidget::CombineWidget(QString title, QString X_title, int AXISX_MIN, int AXISX_MAX, QString Y_title, int AXISY_MIN, int AXISY_MAX, QWidget* parent): QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->addWidget(layoutAdjust = new QWidget, 1);
    layoutAdjust->setVisible(false);
    layout->addWidget(chartWaterfall = new ChartViewWaterfall(title, X_title, AXISX_MIN, AXISX_MAX, Y_title, AXISY_MIN, AXISY_MAX), 10);
    layout->addWidget(chartSpectrum = new ChartViewSpectrum(title, X_title, AXISX_MIN, AXISX_MAX, Y_title, AXISY_MIN, AXISY_MAX), 10);
}
