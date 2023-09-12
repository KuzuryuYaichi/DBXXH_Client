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

    QColor BoundBrushColor(255, 50, 30, 50);
    BoundSeries = addGraph();
    BoundSeries->setName("Bound");
    BoundSeries->setPen(QPen(BoundBrushColor));
    BoundSeries->setLineStyle(QCPGraph::lsLine);
    BoundSeries->rescaleAxes(true);
    BoundSeries->setData({ MID_FREQ - 75e-3, MID_FREQ - 75e-3, MID_FREQ + 75e-3, MID_FREQ + 75e-3 }, { MAX_AMPL, MIN_AMPL, MIN_AMPL, MAX_AMPL }, true);
    BoundSeries->setBrush(QBrush(BoundBrushColor));

    QColor RectBrushColor(0, 0, 255, 50);
    RectSeries = addGraph();
    RectSeries->setName("Rect");
    RectSeries->setPen(QPen(RectBrushColor));
    RectSeries->setLineStyle(QCPGraph::lsLine);
    RectSeries->rescaleAxes(true);
    RectSeries->setBrush(QBrush(RectBrushColor));

//    legend->setVisible(true);
    legend->setBorderPen(Qt::NoPen);
    legend->item(3)->setVisible(false);
    legend->setFont(QFont("YaHei", 9));
    legend->setIconSize(5, 5);

    tracer = new QCPItemTracer(this);
    tracer->setPen(QPen(Qt::gray));
    tracer->setBrush(QBrush(Qt::gray));
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setSize(5);

    for (auto i = 0; i < MARKER_NUM; ++i)
    {
        TracerMarker[i] = new QCPItemTracer(this);
        TracerMarker[i]->setPen(QPen(MARKER_COLOR[i]));
        TracerMarker[i]->setBrush(QBrush(MARKER_COLOR[i]));
        TracerMarker[i]->setStyle(QCPItemTracer::tsCircle);
        TracerMarker[i]->setSize(5);
        TracerMarker[i]->setInterpolating(true);
        TracerMarker[i]->setGraph(SpectrumSeries);
    }

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event) {
        UpdateRect(event);
        UpdateRuler(event);
        UpdateTracer(event);
        replot();
    });

    connect(this, &QCustomPlot::mousePress, this, [this](QMouseEvent *event) {
        auto x = xAxis->pixelToCoord(event->pos().x());
        auto y = yAxis->pixelToCoord(event->pos().y());
        if (event->button() & Qt::RightButton)
        {
            if (xAxis->range().contains(x) && yAxis->range().contains(y))
                RightButtonPress = true;
        }
        if (event->button() & Qt::LeftButton)
        {
            if (xAxis->range().contains(x) && yAxis->range().contains(y))
            {
                LeftButtonPress = true;
                RectStartValue = x;
            }
        }
        replot();
    });

    connect(this, &QCustomPlot::mouseRelease, this, [this](QMouseEvent *event) {
        if (event->button() & Qt::RightButton)
        {
            RightButtonPress = false;
//            emit thresholdEnterPressedSignal(GateSeries->data().data()->at(0)->value);
        }
        if (event->button() & Qt::LeftButton)
        {
            LeftButtonPress = false;
        }
        replot();
    });

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](QPoint pos) {
        auto menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        auto addMenu = new QMenu(tr("Add"), menu);
        menu->addMenu(addMenu);
        for (int i = 0; i < MARKER_NUM; ++i)
        {
            QPixmap pixmap(100, 100);
            pixmap.fill(MARKER_COLOR[i]);
            auto action = new QAction(QIcon(pixmap), tr("Marker %1").arg(i + 1), addMenu);
            addMenu->addAction(action);
            connect(action, &QAction::triggered, this, [this, i] {

            });
        }
        auto removeMenu = new QMenu(tr("Delete"), menu);
        menu->addMenu(removeMenu);
        for (int i = 0; i < MARKER_NUM; ++i)
        {
            QPixmap pixmap(100, 100);
            pixmap.fill(MARKER_COLOR[i]);
            auto action = new QAction(QIcon(pixmap), tr("Marker %1").arg(i + 1), removeMenu);
            removeMenu->addAction(action);
            connect(action, &QAction::triggered, this, [this, i] {

            });
        }

        menu->popup(mapToGlobal(pos));
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

void ChartViewSpectrum::analyzeFrame(size_t DataPoint)
{
    if (pointsMax.size() != DataPoint)
    {
        pointsMax.resize(DataPoint);
        pointsMin.resize(DataPoint);
        for (auto i = 0; i < DataPoint; ++i)
        {
            pointsMax[i] = MIN_AMPL;
            pointsMin[i] = 0;
        }
    }
}

void ChartViewSpectrum::UpdateRect(QMouseEvent *event)
{
    if (LeftButtonPress)
    {
        auto xValue = xAxis->pixelToCoord(event->pos().x());
        QVector<double> x{ RectStartValue, RectStartValue, xValue, xValue }, y{ MAX_AMPL, MIN_AMPL, MIN_AMPL, MAX_AMPL };
        RectSeries->setData(x, y, true);
    }
}

void ChartViewSpectrum::UpdateRuler(QMouseEvent *event)
{
    if (RightButtonPress)
    {
        auto xValue = xAxis->pixelToCoord(event->pos().x());
        QVector<double> x{ xValue - 75e-3, xValue - 75e-3, xValue + 75e-3, xValue + 75e-3 }, y{ MAX_AMPL, MIN_AMPL, MIN_AMPL, MAX_AMPL };
        BoundSeries->setData(x, y, true);
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

void ChartViewSpectrum::SeriesSelectChanged(bool MaxKeepSelect, bool MinKeepSelect, bool SpectrumSelect)
{
    MaxKeepSeries->setVisible(this->MaxKeepSelect = MaxKeepSelect);
    MinKeepSeries->setVisible(this->MinKeepSelect = MinKeepSelect);
    SpectrumSeries->setVisible(this->SpectrumSelect = SpectrumSelect);
}

void ChartViewSpectrum::rescaleKeyAxis(const QCPRange& range)
{
    if (range != xRange)
    {
        SpectrumSeries->rescaleKeyAxis();
    }
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
        auto BAND_WIDTH = (param->StopFreq - param->StartFreq) / 1e6;
        auto freq_step = ResolveResolution(param->Resolution, BAND_WIDTH);
        auto start_freq = param->StartFreq / 1e6;
        auto amplData = (unsigned char*)(buf + sizeof(DataHead) + sizeof(ParamPowerWB));
        analyzeFrame(param->DataPoint);
        QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
        auto x = start_freq;
        for (int i = 0; i < param->DataPoint; ++i)
        {
            amplx[i] = x;
            double y = (short)amplData[i] + AMPL_OFFSET;
            amply[i] = y;
            pointsMax[i] = std::max(y, pointsMax[i]);
            pointsMin[i] = std::min(y, pointsMin[i]);
            x += freq_step;
        }
        if (SpectrumSelect)
            SpectrumSeries->setData(amplx, amply, true);
        if (MaxKeepSelect)
            MaxKeepSeries->setData(amplx, pointsMax, true);
        if (MinKeepSelect)
            MinKeepSeries->setData(amplx, pointsMin, true);
        QCPRange range(param->StartFreq / 1e6, param->StopFreq / 1e6);
        xRangeChanged(range);
        rescaleKeyAxis(range);
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
        SpectrumSeries->setData(amplx, amply);
        QCPRange range(param->Frequency / 1e6 - RealHalfBandWidth, param->Frequency / 1e6 + RealHalfBandWidth);
        xRangeChanged(range);
        rescaleKeyAxis(range);
        break;
    }
    default: return;
    }
    replot(QCustomPlot::rpQueuedReplot);
}
