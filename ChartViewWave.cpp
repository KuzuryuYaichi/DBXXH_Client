#include "ChartViewWave.h"

#include "StructNetData.h"

ChartViewWave::ChartViewWave(QString title, QString X_title, int AXISX_MIN, int AXISX_MAX, QString Y_title, int AXISY_MIN, int AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, X_title, Y_title, parent)
{
    plotLayout()->insertRow(1);
    xAxis->setRange(AXISX_MIN, AXISX_MAX);
    yAxis->setRange(AXISY_MIN, AXISY_MAX);

    ISeries = addGraph();
    ISeries->setPen(QPen(Qt::green));
    ISeries->setLineStyle(QCPGraph::lsLine);
    ISeries->rescaleAxes(true);

    QSeries = addGraph();
    QSeries->setPen(QPen(Qt::red));
    QSeries->setLineStyle(QCPGraph::lsLine);
    QSeries->rescaleAxes(true);

    tracer = new QCPItemTracer(this);
    tracer->setPen(QPen(Qt::red));
    tracer->setBrush(QBrush(Qt::red));
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setSize(5);

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event) {
        UpdateTracer(event);
        replot();
    });
}

void ChartViewWave::UpdateTracer(QMouseEvent *event)
{
    auto x = xAxis->pixelToCoord(event->pos().x());
    tracer->setGraphKey(x);
    tracer->setInterpolating(true);
    tracer->setGraph(graph(0));
    double xValue = tracer->position->key();
    double yValue = tracer->position->value();
    QToolTip::showText(tracer->position->pixelPosition().toPoint(), QString("%1MHz, %2dBm").arg(xValue, 0, 'f', DECIMALS_PRECISION).arg(yValue));
}

void ChartViewWave::replace(unsigned char* const buf)
{
    if (!ready)
        return;
    ready = false;
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x602:
    {
        auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
        auto DataPoint = param->DataPoint;
        auto amplData = (NarrowDDC*)(param + 1);
        QVector<double> amplx(DataPoint), amplyI(DataPoint), amplyQ(DataPoint);
        for (int i = 0; i < DataPoint; ++i)
        {
            amplx[i] = i;
            amplyI[i] = (short)amplData[i].I;
            amplyQ[i] = (short)amplData[i].I;
        }
        ISeries->setData(amplx, amplyI, true);
        QSeries->setData(amplx, amplyQ, true);
        break;
    }
    default: return;
    }
    replot(QCustomPlot::rpQueuedReplot);
}
