#include "ChartViewSpectrum.h"

#include "global.h"
#include "StructNetData.h"

ChartViewSpectrum::ChartViewSpectrum(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, tr("Freq(MHz)"), tr("Ampl(dBm)"), parent)
{
    plotLayout()->insertRow(1);
    plotLayout()->addElement(1, 0, thresholdLbl = new QCPTextElement(this, tr("Gate: ") + "0dBm", QFont("sans", 9, QFont::Bold)));
//    plotLayout()->addElement(1, 1, legend);
    xAxis->setRange(xRange = { AXISX_MIN, AXISX_MAX });
    yAxis->setRange(yRange = { AXISY_MIN, AXISY_MAX });

    SpectrumSeries = addGraph();
    SpectrumSeries->setName(tr("Spectrum"));
    SpectrumSeries->setPen(QPen(Qt::blue));
    SpectrumSeries->setLineStyle(QCPGraph::lsLine);
    SpectrumSeries->rescaleAxes(true);

    MaxKeepSeries = addGraph();
    MaxKeepSeries->setName(tr("Max Keep"));
    MaxKeepSeries->setPen(QPen(Qt::green));
    MaxKeepSeries->setLineStyle(QCPGraph::lsLine);
    MaxKeepSeries->rescaleAxes(true);

    MinKeepSeries = addGraph();
    MinKeepSeries->setName(tr("Min Keep"));
    MinKeepSeries->setPen(QPen(Qt::yellow));
    MinKeepSeries->setLineStyle(QCPGraph::lsLine);
    MinKeepSeries->rescaleAxes(true);

    GateSeries = addGraph();
    GateSeries->setName(tr("Gate"));
    GateSeries->setPen(QPen(Qt::red));
    GateSeries->setLineStyle(QCPGraph::lsLine);
    GateSeries->rescaleAxes(true);
    QVector<double> x {MIN_FREQ, MAX_FREQ}, y {0, 0};
    GateSeries->setData(x, y);

//    legend->setVisible(true);
    legend->setBorderPen(Qt::NoPen);
    legend->item(3)->setVisible(false);
    legend->setFont(QFont("YaHei", 9));
    legend->setIconSize(5, 5);

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
            auto threshold = GateSeries->data().data()->at(0)->value;
            thresholdLbl->setText(tr("Gate: ") + QString::number(threshold) + "dBm");
            emit thresholdEnterPressedSignal(threshold);
        }
        replot();
    });

    inR = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * DDC_LEN);
    outR = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * DDC_LEN);
    planR = fftw_plan_dft_1d(DDC_LEN, inR, outR, FFTW_FORWARD, FFTW_MEASURE);
}

ChartViewSpectrum::~ChartViewSpectrum()
{
    fftw_destroy_plan(planR);
    fftw_free(inR);
    fftw_free(outR);
}

void ChartViewSpectrum::analyzeFrame(unsigned char* amplData, size_t DataPoint)
{
    if (pointsMax.size() != DataPoint)
    {
        pointsMax.resize(DataPoint);
        for (auto i = 0; i < DataPoint; ++i)
            pointsMax[i] = MIN_AMPL;
    }
    for (auto i = 0ull; i < DataPoint; ++i)
    {
        pointsMax[i] = std::max((double)((short)amplData[i] + AMPL_OFFSET), pointsMax[i]);
    }
}

void ChartViewSpectrum::UpdateRuler(QMouseEvent *event)
{
    if (isPress)
    {
        auto yValue = yAxis->pixelToCoord(event->pos().y());
        QVector<double> x(2), y(2);
        x[0] = MIN_FREQ; x[1] = MAX_FREQ;
        y[0] = yValue; y[1] = yValue;
        GateSeries->setData(x, y, true);
    }
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

void ChartViewSpectrum::replace(unsigned char* const buf)
{
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x515:
    {
        auto param = (ParamPowerWB*)(buf + sizeof(DataHead));
        auto data = buf + sizeof(DataHead) + sizeof(ParamPowerWB);
        auto freq_step = param->Resolution / 1e3, start_freq = param->StartFreq / 1e6;
        auto amplData = (unsigned char*)(data + sizeof(long long));
//        analyzeFrame(amplData, param->DataPoint);
        QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
        auto x = start_freq;
        for (int i = 0; i < param->DataPoint; ++i)
        {
            amplx[i] = x;
            amply[i] = (short)amplData[i] + AMPL_OFFSET;
            x += freq_step;
        }
        if (!ready)
            return;
        ready = false;
        SpectrumSeries->setData(amplx, amply, true);
//        MaxKeepSeries->setData(amplx, pointsMax, true);
        break;
    }
    case 0x602:
    {
        auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
        auto data = (NarrowDDC*)(param + 1);
        static const auto WINDOW = HanningWindow<DDC_LEN>();
        for (auto p = 0; p < param->DataPoint; ++p)
        {
            inR[p][0] = data[p].I * WINDOW[p];
            inR[p][1] = data[p].Q * WINDOW[p];
        }
        fftw_execute(planR);
        const auto HALF_LEN = param->DataPoint / 2;
        for (int p = 0; p < HALF_LEN; ++p)
        {
            std::swap(outR[p], outR[HALF_LEN + p]);
        }
        auto RealHalfBandWidth = NB_HALF_BANDWIDTH[11];
        switch (param->BandWidth)
        {
        case 150: RealHalfBandWidth = NB_HALF_BANDWIDTH[0]; break;
        case 300: RealHalfBandWidth = NB_HALF_BANDWIDTH[1]; break;
        case 600: RealHalfBandWidth = NB_HALF_BANDWIDTH[2]; break;
        case 1500: RealHalfBandWidth = NB_HALF_BANDWIDTH[3]; break;
        case 2400: RealHalfBandWidth = NB_HALF_BANDWIDTH[4]; break;
        case 6000: RealHalfBandWidth = NB_HALF_BANDWIDTH[5]; break;
        case 9000: RealHalfBandWidth = NB_HALF_BANDWIDTH[6]; break;
        case 15000: RealHalfBandWidth = NB_HALF_BANDWIDTH[7]; break;
        case 30000: RealHalfBandWidth = NB_HALF_BANDWIDTH[8]; break;
        case 50000: RealHalfBandWidth = NB_HALF_BANDWIDTH[9]; break;
        case 120000: RealHalfBandWidth = NB_HALF_BANDWIDTH[10]; break;
        case 150000: RealHalfBandWidth = NB_HALF_BANDWIDTH[11]; break;
        }
        double freq = param->Frequency / 1e6 - RealHalfBandWidth, step = RealHalfBandWidth * 2 / param->DataPoint;
        QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
        for (int p = 0; p < param->DataPoint; ++p)
        {
            amply[p] = 20 * std::log10(std::sqrt(std::pow(outR[p][0], 2.0) + std::pow(outR[p][1], 2.0))) + AMPL_OFFSET;
            amplx[p] = freq;
            freq += step;
        }
        if (!ready)
            return;
        ready = false;
        SpectrumSeries->setData(amplx, amply);
        SpectrumSeries->rescaleKeyAxis();
        break;
    }
    default: return;
    }
    replot(QCustomPlot::rpQueuedReplot);
}
