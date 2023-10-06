#include "ChartViewSpectrumNB.h"

#include "StructNetData.h"

ChartViewSpectrumNB::ChartViewSpectrumNB(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewSpectrum(title, AXISX_MIN, AXISX_MAX, AXISY_MIN, AXISY_MAX, parent)
{
    plotLayout()->insertRow(1);
    plotLayout()->addElement(1, 0, ThresholdLbl = new QCPTextElement(this, tr("Record Gate: ") + "0dBm", QFont("sans", 9, QFont::Bold)));

    GateSeries = addGraph();
    GateSeries->setName("Gate");
    GateSeries->setPen(QPen(Qt::red));
    GateSeries->setLineStyle(QCPGraph::lsLine);
    GateSeries->rescaleAxes(true);
    GateSeries->setData({ MIN_FREQ, MAX_FREQ }, {0, 0});

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event) {
        UpdateRuler(event);
        UpdateThreshold();
        replot();
    });

    connect(this, &QCustomPlot::mousePress, this, [this](QMouseEvent *event) {
        if (event->button() == Qt::LeftButton)
        {
            auto x = xAxis->pixelToCoord(event->pos().x());
            auto y = yAxis->pixelToCoord(event->pos().y());
            if (xAxis->range().contains(x) && yAxis->range().contains(y))
            {
                LeftButtonPress = true;
            }
        }
        replot();
    });

    connect(this, &QCustomPlot::mouseRelease, this, [this](QMouseEvent *event) {
        if (event->button() == Qt::LeftButton)
        {
            LeftButtonPress = false;
            emit RecordThresholdSignal(GateSeries->data().data()->at(0)->value);
        }
        replot();
    });
}

void ChartViewSpectrumNB::UpdateThreshold()
{
    if (LeftButtonPress)
    {
        auto threshold = GateSeries->data().data()->at(0)->value;
        ThresholdLbl->setText(tr("Record Gate: ") + QString::number(threshold) + "dBm");
    }
}

void ChartViewSpectrumNB::UpdateRuler(QMouseEvent *event)
{
    if (LeftButtonPress)
    {
        auto yValue = yAxis->pixelToCoord(event->pos().y());
        QVector<double> x(2), y(2);
        x[0] = MIN_FREQ; x[1] = MAX_FREQ;
        y[0] = yValue; y[1] = yValue;
        GateSeries->setData(x, y, true);
    }
}

void ChartViewSpectrumNB::replace(unsigned char* const buf, unsigned char* fft_data)
{
    if (!ready)
        return;
    ready = false;
    auto param = (StructNBWave*)(buf + sizeof(DataHead));
    double HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[11], HalfBandwidth = param->Bandwidth / 1e6 / 2;
    switch (param->Bandwidth)
    {
    case 150: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[0]; break;
    case 300: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[1]; break;
    case 600: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[2]; break;
    case 1500: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[3]; break;
    case 2400: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[4]; break;
    case 6000: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[5]; break;
    case 9000: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[6]; break;
    case 15000: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[7]; break;
    case 30000: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[8]; break;
    case 50000: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[9]; break;
    case 120000: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[10]; break;
    case 150000: HalfSpsBandwidth = NB_HALF_BANDWIDTH_MHz[11]; break;
    }
    double mid_freq = param->Frequency / 1e6;
    double freq = param->Frequency / 1e6 - HalfSpsBandwidth, step = HalfSpsBandwidth * 2 / param->DataPoint;
    QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
    for (int p = 0; p < param->DataPoint; ++p)
    {
        amply[p] = (short)fft_data[p] + AMPL_OFFSET;
        amplx[p] = freq;
        freq += step;
    }
    SpectrumSeries->setData(amplx, amply);
    BandwidthSeries->setData({ mid_freq - HalfBandwidth, mid_freq - HalfBandwidth, mid_freq + HalfBandwidth, mid_freq + HalfBandwidth }, { MAX_AMPL, MIN_AMPL, MIN_AMPL, MAX_AMPL }, true);
    QCPRange range(param->Frequency / 1e6 - HalfSpsBandwidth, param->Frequency / 1e6 + HalfSpsBandwidth);
    xRangeChanged(range);
    replot(QCustomPlot::rpQueuedReplot);
}
