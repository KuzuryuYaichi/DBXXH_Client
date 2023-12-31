#include "ChartViewWave.h"

#include "StructNetData.h"

ChartViewWave::ChartViewWave(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Points"), tr("Power"), parent)
{
    xAxis->setRange(xRange = { AXISX_MIN, AXISX_MAX });
    yAxis->setRange(yRange = { AXISY_MIN, AXISY_MAX });

    ISeries = addGraph();
    ISeries->setPen(QPen(Qt::red));
    ISeries->setLineStyle(QCPGraph::lsLine);
    ISeries->setScatterStyle(QCPScatterStyle::ssCircle);
    ISeries->rescaleAxes(true);

    QSeries = addGraph();
    QSeries->setPen(QPen(Qt::green));
    QSeries->setLineStyle(QCPGraph::lsLine);
    QSeries->setScatterStyle(QCPScatterStyle::ssCircle);
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
    QToolTip::showText(mapToGlobal(tracer->position->pixelPosition().toPoint()), QString("%1MHz, %2").arg(xValue, 0, 'f', DECIMALS_PRECISION).arg(yValue));
}

void ChartViewWave::replace(unsigned char* const buf)
{
    if (!ready)
        return;
    ready = false;
    auto param = (StructNBWave*)(buf + sizeof(DataHead));
    auto DataPoint = param->DataPoint;
    auto amplData = (NarrowDDC*)(param + 1);
    QVector<double> amplx(DataPoint), amplyI(DataPoint), amplyQ(DataPoint);
    for (int i = 0; i < DataPoint; ++i)
    {
        amplx[i] = i;
        amplyI[i] = amplData[i].I;
        amplyQ[i] = amplData[i].Q;
    }
    ISeries->setData(amplx, amplyI, true);
    QSeries->setData(amplx, amplyQ, true);
    replot(QCustomPlot::rpQueuedReplot);
}
