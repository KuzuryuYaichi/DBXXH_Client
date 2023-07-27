#include "ChartViewCustom.h"

ChartViewCustom::ChartViewCustom(QString title, QString X_title, QString Y_title, QWidget* parent): QCustomPlot(parent)
{
    plotLayout()->insertRow(0);
    plotLayout()->addElement(0, 0, new QCPTextElement(this, title, QFont("sans", 12, QFont::Bold)));
    xAxis->setLabel(X_title);
    yAxis->setLabel(Y_title);
    xAxis2->setVisible(false);
    yAxis2->setVisible(false);
    setInteractions(QCP::iRangeZoom);
    setInteractions(QCP::iRangeZoom);

    connect(this, &QCustomPlot::mouseDoubleClick, this, [this](QMouseEvent *event) {
        if (event->button() == Qt::LeftButton)
        {
            xAxis->setRange(xMin, xMax);
            yAxis->setRange(yMin, yMax);
        }
        replot(QCustomPlot::rpQueuedReplot);
    });

    m_updater = new QTimer;
    m_updater->setInterval(TIME_INTERVAL);
    m_updater->setSingleShot(true);
    connect(m_updater, &QTimer::timeout, this, [this] {
        ready = true;
    });
    connect(this, &QCustomPlot::afterReplot, this, [this] {
        m_updater->start();
    });
}

void ChartViewCustom::xCenterChanged(double min, double max)
{
    xAxis->setRange(xMin = min, xMax = max);
    replot(QCustomPlot::rpQueuedReplot);
}

void ChartViewCustom::yCenterChanged(double min, double max)
{
    yAxis->setRange(yMin = min, yMax = max);
    replot(QCustomPlot::rpQueuedReplot);
}

void ChartViewCustom::setAxisxMin(double min)
{
    xAxis->setRangeLower(xMin = min);
    replot(QCustomPlot::rpQueuedReplot);
}

void ChartViewCustom::setAxisxMax(double max)
{
    xAxis->setRangeUpper(xMax = max);
    replot(QCustomPlot::rpQueuedReplot);
}

void ChartViewCustom::setAxisyMin(double min)
{
    yAxis->setRangeLower(yMin = min);
    replot(QCustomPlot::rpQueuedReplot);
}

void ChartViewCustom::setAxisyMax(double max)
{
    yAxis->setRangeUpper(yMax = max);
    replot(QCustomPlot::rpQueuedReplot);
}
