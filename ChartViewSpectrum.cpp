#include "ChartViewSpectrum.h"

#include "StructNetData.h"

ChartViewSpectrum::ChartViewSpectrum(QString title, QString X_title, int AXISX_MIN, int AXISX_MAX, QString Y_title, int AXISY_MIN, int AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, X_title, Y_title, parent)
{
    plotLayout()->insertRow(1);
    plotLayout()->addElement(1, 0, thresholdLbl = new QCPTextElement(this, tr("Gate: ") + "0dBm", QFont("sans", 9, QFont::Bold)));
    xAxis->setRange(AXISX_MIN, AXISX_MAX);
    yAxis->setRange(AXISY_MIN, AXISY_MAX);

    SpectrumSeries = addGraph();
    SpectrumSeries->setPen(QPen(Qt::blue));
    SpectrumSeries->setLineStyle(QCPGraph::lsLine);
    SpectrumSeries->rescaleAxes(true);

    GateSeries = addGraph();
    GateSeries->setPen(QPen(Qt::red));
    GateSeries->setLineStyle(QCPGraph::lsLine);
    GateSeries->rescaleAxes(true);
    QVector<double> x {MIN_SAMPLE_FREQ, MAX_SAMPLE_FREQ}, y {0, 0};
    GateSeries->setData(x, y);

    tracer = new QCPItemTracer(this);
    tracer->setPen(QPen(Qt::red));
    tracer->setBrush(QBrush(Qt::red));
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setSize(5);

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event) {
        UpdateRuler(event);
        UpdateTracer(event);
        replot();
    });

    connect(this, &QCustomPlot::mousePress, this, [this](QMouseEvent *event) {
        if (event->button() == Qt::LeftButton)
        {
            auto x = xAxis->pixelToCoord(event->pos().x());
            auto y = yAxis->pixelToCoord(event->pos().y());
            if (xAxis->range().contains(x) && yAxis->range().contains(y))
                isPress = true;
        }
        replot();
    });

    connect(this, &QCustomPlot::mouseRelease, this, [this](QMouseEvent *event) {
        if (event->button() == Qt::LeftButton)
        {
            isPress = false;
            auto threshold = graph(1)->data().data()->at(0)->value;
            thresholdLbl->setText(tr("Gate: ") + QString::number(threshold) + "dBm");
            emit thresholdEnterPressedSignal(threshold);
        }
        replot();
    });
}

void ChartViewSpectrum::UpdateRuler(QMouseEvent *event)
{
    if (isPress)
    {
        auto yValue = yAxis->pixelToCoord(event->pos().y());
        QVector<double> x(2), y(2);
        x[0] = MIN_FREQ; x[1] = MAX_FREQ;
        y[0] = yValue; y[1] = yValue;
        graph(1)->setData(x, y, true);
    }
}

void ChartViewSpectrum::UpdateTracer(QMouseEvent *event)
{
    auto x = xAxis->pixelToCoord(event->pos().x());
    tracer->setGraphKey(x);
    tracer->setInterpolating(true);
    tracer->setGraph(graph(0));
    double xValue = tracer->position->key();
    double yValue = tracer->position->value();
    QToolTip::showText(tracer->position->pixelPosition().toPoint(), QString("%1MHz, %2dBm").arg(xValue, 0, 'f', DECIMALS_PRECISION).arg(yValue));
}

void ChartViewSpectrum::replace(unsigned char* const buf)
{
    if (!ready)
        return;
    ready = false;
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x515:
    {
        auto param = (ParamPowerWB*)(buf + sizeof(DataHead));
        auto DataPoint = param->DataPoint;
        const auto GROUP_LENGTH = sizeof(long long) + (sizeof(char) + sizeof(short)) * DataPoint;
        auto data = buf + sizeof(DataHead) + sizeof(ParamPowerWB);
        auto freq_step = param->Resolution / 1e3, start_freq = param->StartFreq / 1e6;
        QVector<double> amplx(DataPoint), amply(DataPoint);
        for (int g = 0; g < 1; ++g)
        {
            auto amplData = (unsigned char*)(data + sizeof(long long));
            auto x = start_freq;
            for (int i = 0; i < DataPoint; ++i)
            {
                auto range = (short)amplData[i] + AMPL_OFFSET;
                amplx[i] = x; amply[i] = range;
                x += freq_step;
            }
            data += GROUP_LENGTH;
        }
        graph(0)->setData(amplx, amply, true);
        break;
    }
    case 0x602:
    {
        auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
        auto data = (NarrowDDC*)(param + 1);
        for (auto p = 0; p < param->DataPoint; ++p)
        {
            inR[p][0] = data[p].I;
            inR[p][1] = data[p].Q;
        }
        fftw_execute(planR);
        const auto HALF_LEN = param->DataPoint / 2;
        for (int p = 0; p < HALF_LEN; ++p)
        {
            std::swap(outR[p], outR[HALF_LEN + p]);
        }
        auto RealHalfBandWidth = NB_HALF_BANDWIDTH[0];
        switch (param->BandWidth)
        {
        case 2400: RealHalfBandWidth = NB_HALF_BANDWIDTH[0]; break;
        case 4800: RealHalfBandWidth = NB_HALF_BANDWIDTH[1]; break;
        case 9600: RealHalfBandWidth = NB_HALF_BANDWIDTH[2]; break;
        case 19200: RealHalfBandWidth = NB_HALF_BANDWIDTH[3]; break;
        case 38400: RealHalfBandWidth = NB_HALF_BANDWIDTH[4]; break;
        case 76800: RealHalfBandWidth = NB_HALF_BANDWIDTH[5]; break;
        case 96000: RealHalfBandWidth = NB_HALF_BANDWIDTH[6]; break;
        }
        double freq = param->Frequency / 1e6 - RealHalfBandWidth, step = RealHalfBandWidth * 2 / param->DataPoint;
        QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
        for (int p = 0; p < param->DataPoint; ++p)
        {
            amply[p] = 20 * std::log10(std::sqrt(std::pow(outR[p][0], 2.0) + std::pow(outR[p][1], 2.0))) + AMPL_OFFSET;
            amplx[p] = freq;
            freq += step;
        }
    }
    default: return;
    }
    replot(QCustomPlot::rpQueuedReplot);
}
