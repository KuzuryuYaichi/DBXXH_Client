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
    xAxis->setRange(xRange = {min, max});
    replot(QCustomPlot::rpQueuedReplot);
}

void ChartViewCustom::yCenterChanged(double min, double max)
{
    yAxis->setRange(yRange = {min, max});
    replot(QCustomPlot::rpQueuedReplot);
}

double ChartViewCustom::ResolveResolution(int Resolution, double BAND_WIDTH)
{
    switch (Resolution)
    {
    case 0x0E:
    {
        if (BAND_WIDTH == 0.9375) return 0.092 / 1e3;
        if (BAND_WIDTH == 1.875)  return 0.183 / 1e3;
        if (BAND_WIDTH == 3.75)   return 0.366 / 1e3;
        if (BAND_WIDTH == 7.5)    return 0.732 / 1e3;
        if (BAND_WIDTH == 15)     return 1.465 / 1e3;
        if (BAND_WIDTH == 30)     return 2.93 / 1e3;
        break;
    }
    case 0x0D:
    {
        if (BAND_WIDTH == 0.9375) return 0.183 / 1e3;
        if (BAND_WIDTH == 1.875)  return 0.366 / 1e3;
        if (BAND_WIDTH == 3.75)   return 0.732 / 1e3;
        if (BAND_WIDTH == 7.5)    return 1.465 / 1e3;
        if (BAND_WIDTH == 15)     return 2.93 / 1e3;
        if (BAND_WIDTH == 30)     return 5.859 / 1e3;
        break;
    }
    case 0x0C:
    {
        if (BAND_WIDTH == 0.9375) return 0.366 / 1e3;
        if (BAND_WIDTH == 1.875)  return 0.732 / 1e3;
        if (BAND_WIDTH == 3.75)   return 1.465 / 1e3;
        if (BAND_WIDTH == 7.5)    return 2.93 / 1e3;
        if (BAND_WIDTH == 15)     return 5.859 / 1e3;
        if (BAND_WIDTH == 30)     return 11.719 / 1e3;
        break;
    }
    case 0x0B:
    {
        if (BAND_WIDTH == 0.9375) return 0.732 / 1e3;
        if (BAND_WIDTH == 1.875)  return 1.465 / 1e3;
        if (BAND_WIDTH == 3.75)   return 2.93 / 1e3;
        if (BAND_WIDTH == 7.5)    return 5.859 / 1e3;
        if (BAND_WIDTH == 15)     return 11.719 / 1e3;
        if (BAND_WIDTH == 30)     return 23.438 / 1e3;
        break;
    }
    case 0x0A:
    {
        if (BAND_WIDTH == 0.9375) return 1.465 / 1e3;
        if (BAND_WIDTH == 1.875)  return 2.93 / 1e3;
        if (BAND_WIDTH == 3.75)   return 5.859 / 1e3;
        if (BAND_WIDTH == 7.5)    return 11.719 / 1e3;
        if (BAND_WIDTH == 15)     return 23.438 / 1e3;
        if (BAND_WIDTH == 30)     return 46.875 / 1e3;
        break;
    }
    }
    return 0;
}
