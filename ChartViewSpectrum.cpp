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

    QColor BandwidthBrushColor(0, 255, 0, 50);
    BandwidthSeries = addGraph();
    BandwidthSeries->setName("Bandwidth");
    BandwidthSeries->setPen(QPen(BandwidthBrushColor));
    BandwidthSeries->setLineStyle(QCPGraph::lsLine);
    BandwidthSeries->rescaleAxes(true);
    BandwidthSeries->setBrush(QBrush(BandwidthBrushColor));

    TracerNormal = new QCPItemTracer(this);
    TracerNormal->setPen(QPen(Qt::black));
    TracerNormal->setBrush(QBrush(Qt::black));
    TracerNormal->setStyle(QCPItemTracer::tsCircle);
    TracerNormal->setSize(8);

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event) {
        UpdateTracer(event);
    });
}

void ChartViewSpectrum::UpdateTracer(QMouseEvent *event)
{
    auto x = xAxis->pixelToCoord(event->pos().x());
    TracerNormal->setGraphKey(x);
    TracerNormal->setInterpolating(true);
    TracerNormal->setGraph(SpectrumSeries);
    double xValue = TracerNormal->position->key();
    double yValue = TracerNormal->position->value();
    QToolTip::showText(mapToGlobal(TracerNormal->position->pixelPosition().toPoint()), QString("%1MHz, %2dBm").arg(xValue, 0, 'f', DECIMALS_PRECISION).arg(yValue));
}
