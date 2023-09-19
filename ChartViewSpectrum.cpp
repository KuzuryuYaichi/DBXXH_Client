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
    TrackSeries = addGraph();
    TrackSeries->setName("Track");
    TrackSeries->setPen(QPen(RectBrushColor));
    TrackSeries->setLineStyle(QCPGraph::lsLine);
    TrackSeries->rescaleAxes(true);
    TrackSeries->setBrush(QBrush(RectBrushColor));

    plotLayout()->insertRow(1);
    plotLayout()->addElement(1, 0, MarkElement = new QCPLayoutGrid);
    for (auto i = 0; i < MARKER_NUM; ++i)
    {
        auto textElement = new QCPTextElement(this, tr("Marker %1").arg(i + 1), QFont("sans", 8, QFont::Bold));
        textElement->setTextColor(MARKER_COLOR[i]);
        MarkElement->addElement(0, i, textElement);
    }
    MarkElement->setVisible(false);

    TracerNormal = new QCPItemTracer(this);
    TracerNormal->setPen(QPen(Qt::black));
    TracerNormal->setBrush(QBrush(Qt::black));
    TracerNormal->setStyle(QCPItemTracer::tsCircle);
    TracerNormal->setSize(8);
    tracer = TracerNormal;

    for (auto i = 0; i < MARKER_NUM; ++i)
    {
        TracerMarker[i] = new QCPItemTracer(this);
        TracerMarker[i]->setPen(QPen(MARKER_COLOR[i]));
        TracerMarker[i]->setBrush(QBrush(MARKER_COLOR[i]));
        TracerMarker[i]->setStyle(QCPItemTracer::tsSquare);
        TracerMarker[i]->setSize(8);
        TracerMarker[i]->setInterpolating(true);
        TracerMarker[i]->setGraph(SpectrumSeries);
        TracerMarker[i]->setVisible(false);

        TracerText[i] = new QCPItemText(this);
        TracerText[i]->setLayer("legend");
        TracerText[i]->setText(QString("M%1").arg(i + 1));
        TracerText[i]->setTextAlignment(Qt::AlignCenter);
        TracerText[i]->setColor(MARKER_COLOR[i]);
        TracerText[i]->setPositionAlignment(Qt::AlignTop);
        TracerText[i]->position->setParentAnchor(TracerMarker[i]->position);
        TracerText[i]->setVisible(false);
    }

    InitMenu();
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](QPoint pos) {
        if (MenuAppear)
            menu->popup(mapToGlobal(pos));
        MenuAppear = true;
    });

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event) {
        UpdateTrack(event);
        UpdateRuler(event);
        UpdateTracer(event);
        replot();
    });

    connect(this, &QCustomPlot::mousePress, this, [this](QMouseEvent *event) {
        if (!(event->button() & Qt::LeftButton))
            return;
        auto x = xAxis->pixelToCoord(event->pos().x());
        auto y = yAxis->pixelToCoord(event->pos().y());
        if (!(xAxis->range().contains(x) && yAxis->range().contains(y)))
            return;
        switch (DisplayState)
        {
        case NORMAL:
        {
            break;
        }
        case TRACK:
//        {
//            LeftButtonPress = true;
//            break;
//        }
        case MEASURE:
        {
            TrackStartFreq = x;
            LeftButtonPress = true;
            break;
        }
        case MARK:
        {
            break;
        }
        default: return;
        }
        replot();
    });

    connect(this, &QCustomPlot::mouseRelease, this, [this](QMouseEvent *event) {
        if (event->button() & Qt::RightButton && DisplayState != NORMAL)
        {
            if (DisplayState == MARK)
            {
                tracer->setVisible(false);
                tracer = TracerNormal;
                tracer->setVisible(true);
            }
            DisplayState = NORMAL;
            MenuAppear = false;
        }
        else if (event->button() & Qt::LeftButton)
        {
            switch (DisplayState)
            {
            case NORMAL:
            {
                LeftButtonPress = false;
                //            emit thresholdEnterPressedSignal(GateSeries->data().data()->at(0)->value);
                break;
            }
            case TRACK:
//            {
//                LeftButtonPress = false;
//                break;
//            }
            case MEASURE:
            {
                TrackEndFreq = xAxis->pixelToCoord(event->pos().x());
                LeftButtonPress = false;
                break;
            }
            case MARK:
            {
                if (event->button() & Qt::RightButton)
                {
                    tracer->setVisible(false);
                }
                tracer = TracerNormal;
                DisplayState = NORMAL;
                break;
            }
            default: return;
            }
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

void ChartViewSpectrum::SaveSpectrum(const QString& path)
{
    MarkElement->setVisible(true);
    replot(QCustomPlot::rpQueuedReplot);
    savePng(path);
    MarkElement->setVisible(false);
}

void ChartViewSpectrum::InitMenu()
{
    menu = new QMenu(this);
    auto markerMenu = new QMenu(tr("Mark"));
    menu->addMenu(markerMenu);

    auto addMenu = new QMenu(tr("Add"));
    markerMenu->addMenu(addMenu);
    for (int i = 0; i < MARKER_NUM; ++i)
    {
        QPixmap pixmap(100, 100);
        pixmap.fill(MARKER_COLOR[i]);
        auto action = new QAction(QIcon(pixmap), tr("Marker %1").arg(i + 1));
        addMenu->addAction(action);
        connect(action, &QAction::triggered, this, [this, i] {
            DisplayState = MARK;
            TracerMarker[i]->setVisible(true);
            TracerText[i]->setVisible(true);
            tracer = TracerMarker[i];
        });
    }
    auto removeMenu = new QMenu(tr("Delete"));
    markerMenu->addMenu(removeMenu);
    for (int i = 0; i < MARKER_NUM; ++i)
    {
        QPixmap pixmap(100, 100);
        pixmap.fill(MARKER_COLOR[i]);
        auto action = new QAction(QIcon(pixmap), tr("Marker %1").arg(i + 1));
        removeMenu->addAction(action);
        connect(action, &QAction::triggered, this, [this, i] {
            DisplayState = NORMAL;
            TracerMarker[i]->setVisible(false);
            TracerText[i]->setVisible(false);
            tracer = TracerNormal;
        });
    }

    auto measureAction = new QAction(tr("Measure"));
    menu->addAction(measureAction);
    connect(measureAction, &QAction::triggered, this, [this] {
        DisplayState = MEASURE;
    });

    auto trackAction = new QAction(tr("Track"));
    menu->addAction(trackAction);
    connect(trackAction, &QAction::triggered, this, [this] {
        DisplayState = TRACK;
    });
}

void ChartViewSpectrum::AnalyzeFrame(size_t DataPoint)
{
    if (pointsMax.size() != DataPoint)
    {
        pointsMax.resize(DataPoint);
        pointsMin.resize(DataPoint);
        for (auto i = 0ull; i < DataPoint; ++i)
        {
            pointsMax[i] = MIN_AMPL;
            pointsMin[i] = 0;
        }
    }
}

void ChartViewSpectrum::UpdateTrack(QMouseEvent *event)
{
    if (DisplayState == TRACK && LeftButtonPress)
    {
        auto xValue = xAxis->pixelToCoord(event->pos().x());
        QVector<double> x{ TrackStartFreq, TrackStartFreq, xValue, xValue }, y{ MAX_AMPL, MIN_AMPL, MIN_AMPL, MAX_AMPL };
        TrackSeries->setData(x, y, true);
    }
}

void ChartViewSpectrum::UpdateRuler(QMouseEvent *event)
{
    if (DisplayState == NORMAL && LeftButtonPress)
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

void ChartViewSpectrum::SeriesSelectChanged(bool MaxKeepSelect, bool MinKeepSelect)
{
    MaxKeepSeries->setVisible(this->MaxKeepSelect = MaxKeepSelect);
    MinKeepSeries->setVisible(this->MinKeepSelect = MinKeepSelect);
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
        auto StartFreq = param->StartFreq / 1e6;
        auto amplData = (unsigned char*)(buf + sizeof(DataHead) + sizeof(ParamPowerWB));
        auto DataPoint = param->DataPoint;
        AnalyzeFrame(DataPoint);
        AnalyzeMark(StartFreq, freq_step, amplData, DataPoint);
        AnalyzeMeasure(StartFreq, freq_step, DataPoint);
        AnalyzeTrack(StartFreq, freq_step, amplData, DataPoint);

        QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
        auto x = StartFreq;
        for (int i = 0; i < param->DataPoint; ++i)
        {
            amplx[i] = x;
            double y = (short)amplData[i] + AMPL_OFFSET;
            amply[i] = y;
            pointsMax[i] = std::max(y, pointsMax[i]);
            pointsMin[i] = std::min(y, pointsMin[i]);
            x += freq_step;
        }
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

void ChartViewSpectrum::AnalyzeMark(double StartFreq, double freq_step, unsigned char* amplData, unsigned int DataPoint)
{
    std::vector<std::tuple<bool, double, double>> MarkData(MARKER_NUM);
    for (auto i = 0; i < MARKER_NUM; ++i)
    {
        auto freq = TracerMarker[i]->graphKey();
        int index = (freq - StartFreq) / freq_step;
        if (index < 0 || index >= DataPoint)
        {
            MarkData[i] = { false, 0, 0 };
            continue;
        }
        auto ampl = (short)amplData[index] + AMPL_OFFSET;
        MarkData[i] = { true, freq, ampl };
        if (TracerMarker[i]->visible())
            ((QCPTextElement*)MarkElement->elementAt(i))->setText(tr("Marker %1: %2dBm @ %3MHz").arg(i + 1).arg(ampl).arg(freq));
        else
            ((QCPTextElement*)MarkElement->elementAt(i))->setText("-");
    }
    emit triggerMark(MarkData);
}

void ChartViewSpectrum::AnalyzeMeasure(double StartFreq, double freq_step, unsigned int DataPoint)
{
    auto LargerFreq = std::max(TrackStartFreq, TrackEndFreq), SmallerFreq = std::min(TrackStartFreq, TrackEndFreq);
    auto indexStart = (LargerFreq - StartFreq) / freq_step;
    if (indexStart <= 0 || indexStart >= DataPoint)
        return;
    auto indexEnd = (SmallerFreq - StartFreq) / freq_step;
    if (indexEnd <= 0 || indexEnd >= DataPoint)
        return;
    emit triggerMeasure(LargerFreq - SmallerFreq);
}

void ChartViewSpectrum::AnalyzeTrack(double StartFreq, double freq_step, unsigned char* amplData, unsigned int DataPoint)
{
    auto LargerFreq = std::max(TrackStartFreq, TrackEndFreq), SmallerFreq = std::min(TrackStartFreq, TrackEndFreq);
    int indexStart = (SmallerFreq - StartFreq) / freq_step;
    if (indexStart < 0 || indexStart >= DataPoint)
        return;
    int indexEnd = (LargerFreq - StartFreq) / freq_step;
    if (indexEnd < 0 || indexEnd >= DataPoint)
        return;
    unsigned long long MaxFreqIndex = indexStart;
    double MaxAmpl = MIN_AMPL;
    for (auto i = indexStart; i <= indexEnd; ++i)
    {
        auto ampl = (short)amplData[i] + AMPL_OFFSET;
        if (ampl > MaxAmpl)
        {
            MaxAmpl = ampl;
            MaxFreqIndex = i;
        }
    }
    emit triggerTrack(StartFreq + freq_step * MaxFreqIndex, MaxAmpl);
}
