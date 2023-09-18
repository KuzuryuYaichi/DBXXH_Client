#include "ChartViewCustom.h"

ChartViewCustom::ChartViewCustom(QString title, QString X_title, QString Y_title, QWidget* parent, int refreshInterval):
    QCustomPlot(parent)
{
    plotLayout()->insertRow(0);
    plotLayout()->addElement(0, 0, new QCPTextElement(this, title, QFont("sans", 12, QFont::Bold)));
    xAxis->setLabel(X_title);
    yAxis->setLabel(Y_title);
    xAxis2->setVisible(false);
    yAxis2->setVisible(false);
    setInteractions(QCP::iRangeZoom);

    connect(this, &QCustomPlot::mouseDoubleClick, this, [this](QMouseEvent *event) {
        if (event->button() == Qt::LeftButton)
        {
            xAxis->setRange(xRange);
            yAxis->setRange(yRange);
        }
        replot();
    });

    m_updater = new QTimer;
    m_updater->setInterval(refreshInterval);
    m_updater->setSingleShot(true);
    connect(m_updater, &QTimer::timeout, this, [this] {
        ready = true;
    });
    connect(this, &QCustomPlot::afterReplot, this, [this] {
        m_updater->start();
    });
}

void ChartViewCustom::xRangeChanged(const QCPRange& range)
{
    if (xRange != range)
    {
        xAxis->setRange(xRange = range);
        replot(QCustomPlot::rpQueuedReplot);
    }
}

void ChartViewCustom::yRangeChanged(const QCPRange& range)
{
    if (yRange != range)
    {
        yAxis->setRange(yRange = range);
        replot(QCustomPlot::rpQueuedReplot);
    }
}

void ChartViewCustom::rescaleKeyAxis(const QCPRange&)
{

}

double ChartViewCustom::ResolveResolution(int Resolution, double BAND_WIDTH)
{
    switch (Resolution)
    {
    case 0x0E:
    {
        if (BAND_WIDTH == 0.9375) return 1.5 / 16384;
        if (BAND_WIDTH == 1.875)  return 3.0 / 16384;
        if (BAND_WIDTH == 3.75)   return 6.0 / 16384;
        if (BAND_WIDTH == 7.5)    return 12.0 / 16384;
        if (BAND_WIDTH == 15)     return 24.0 / 16384;
        if (BAND_WIDTH == 30)     return 48.0 / 16384;
        break;
    }
    case 0x0D:
    {
        if (BAND_WIDTH == 0.9375) return 3.0 / 16384;
        if (BAND_WIDTH == 1.875)  return 6.0 / 16384;
        if (BAND_WIDTH == 3.75)   return 12.0 / 16384;
        if (BAND_WIDTH == 7.5)    return 24.0 / 16384;
        if (BAND_WIDTH == 15)     return 48.0 / 16384;
        if (BAND_WIDTH == 30)     return 96.0 / 16384;
        break;
    }
    case 0x0C:
    {
        if (BAND_WIDTH == 0.9375) return 6.0 / 16384;
        if (BAND_WIDTH == 1.875)  return 12.0 / 16384;
        if (BAND_WIDTH == 3.75)   return 24.0 / 16384;
        if (BAND_WIDTH == 7.5)    return 48.0 / 16384;
        if (BAND_WIDTH == 15)     return 96.0 / 16384;
        if (BAND_WIDTH == 30)     return 192.0 / 16384;
        break;
    }
    case 0x0B:
    {
        if (BAND_WIDTH == 0.9375) return 12.0 / 16384;
        if (BAND_WIDTH == 1.875)  return 24.0 / 16384;
        if (BAND_WIDTH == 3.75)   return 48.0 / 16384;
        if (BAND_WIDTH == 7.5)    return 96.0 / 16384;
        if (BAND_WIDTH == 15)     return 192.0 / 16384;
        if (BAND_WIDTH == 30)     return 384.0 / 16384;
        break;
    }
    case 0x0A:
    {
        if (BAND_WIDTH == 0.9375) return 24.0 / 16384;
        if (BAND_WIDTH == 1.875)  return 48.0 / 16384;
        if (BAND_WIDTH == 3.75)   return 96.0 / 16384;
        if (BAND_WIDTH == 7.5)    return 192.0 / 16384;
        if (BAND_WIDTH == 15)     return 384.0 / 16384;
        if (BAND_WIDTH == 30)     return 46.875 / 1e3;
        break;
    }
    }
    return 0;
}
