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
            xAxis->setRange(xMin, xMax);
            yAxis->setRange(yMin, yMax);
        }
        replot();
    });

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](QPoint pos) {
        auto menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        auto x = xAxis->pixelToCoord(pos.x());
        auto action = new QAction((QString::number(x, 'f', DECIMALS_PRECISION) + "MHz"), this);
        action->setEnabled(false);
        menu->addAction(action);
        menu->popup(mapToGlobal(pos));
        for (int i = 0; i < MARKER_NUM; ++i)
        {
            auto action = new QAction(tr("Add To Marker%1").arg(i + 1), this);
            menu->addAction(action);
//            connect(action, &QAction::triggered, this, [this, i, x] {
//                this->markerView[i]->SetFreqEdit(x * 1e6);
//            });
        }
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
