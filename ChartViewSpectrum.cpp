#include "ChartViewSpectrum.h"
#include "global.h"

ChartViewSpectrum::ChartViewSpectrum(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Freq(MHz)"), tr("Ampl(dBm)"), parent)
{
    xAxis->setRange(xRange = { AXISX_MIN, AXISX_MAX });
    yAxis->setRange(yRange = { AXISY_MIN, AXISY_MAX });

    SpectrumSeries = addGraph();
    SpectrumSeries->setName(tr("Spectrum"));
    SpectrumSeries->setPen(QPen(Qt::blue));
    SpectrumSeries->setLineStyle(QCPGraph::lsLine);
    SpectrumSeries->rescaleAxes(true);

    TracerNormal = new QCPItemTracer(this);
    TracerNormal->setPen(QPen(Qt::black));
    TracerNormal->setBrush(QBrush(Qt::black));
    TracerNormal->setStyle(QCPItemTracer::tsCircle);
    TracerNormal->setSize(8);
    tracer = TracerNormal;

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event) {
        UpdateTracer(event);
    });
}

void ChartViewSpectrum::UpdateTracer(QMouseEvent *event)
{
    auto x = xAxis->pixelToCoord(event->pos().x());
    tracer->setGraphKey(x);
    tracer->setInterpolating(true);
    tracer->setGraph(SpectrumSeries);
    double xValue = tracer->position->key();
    double yValue = tracer->position->value();
    QToolTip::showText(mapToGlobal(tracer->position->pixelPosition().toPoint()), QString("%1MHz, %2dBm").arg(xValue, 0, 'f', DECIMALS_PRECISION).arg(yValue));
}

void ChartViewSpectrum::rescaleKeyAxis(const QCPRange& range)
{
    if (range != xRange)
    {
        SpectrumSeries->rescaleKeyAxis();
    }
}
