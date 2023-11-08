#include "ChartViewSpectrumWB.h"

#include "StructNetData.h"

ChartViewSpectrumWB::ChartViewSpectrumWB(QString title, double AXISX_MIN, double AXISX_MAX, double AXISY_MIN, double AXISY_MAX, QWidget* parent):
    ChartViewSpectrum(title, AXISX_MIN, AXISX_MAX, AXISY_MIN, AXISY_MAX, parent)
{
    MaxKeepSeries = addGraph();
    MaxKeepSeries->setName(tr("Max Keep"));
    MaxKeepSeries->setPen(QPen(Qt::green));
    MaxKeepSeries->setLineStyle(QCPGraph::lsLine);
    MaxKeepSeries->rescaleAxes(true);

    MinKeepSeries = addGraph();
    MinKeepSeries->setName(tr("Min Keep"));
    MinKeepSeries->setPen(QPen(QColor(128, 0, 128)));
    MinKeepSeries->setLineStyle(QCPGraph::lsLine);
    MinKeepSeries->rescaleAxes(true);

    BandwidthSeries->setData({ MID_FREQ - 75e-3, MID_FREQ - 75e-3, MID_FREQ + 75e-3, MID_FREQ + 75e-3 }, { MAX_AMPL, MIN_AMPL, MIN_AMPL, MAX_AMPL }, true);

    QColor TrackColor(0, 0, 255, 50);
    TrackSeries = addGraph();
    TrackSeries->setName("Track");
    TrackSeries->setPen(QPen(TrackColor));
    TrackSeries->setLineStyle(QCPGraph::lsLine);
    TrackSeries->rescaleAxes(true);
    TrackSeries->setBrush(QBrush(TrackColor));

    QColor FM_IndexColor(128, 0, 128, 50);
    FM_IndexSeries = addGraph();
    FM_IndexSeries->setName("FM Index");
    FM_IndexSeries->setPen(QPen(FM_IndexColor));
    FM_IndexSeries->setLineStyle(QCPGraph::lsLine);
    FM_IndexSeries->rescaleAxes(true);
    FM_IndexSeries->setBrush(QBrush(FM_IndexColor));

    plotLayout()->insertRow(1);
    plotLayout()->addElement(1, 0, MarkElement = new QCPLayoutGrid);
    for (auto i = 0; i < MARKER_NUM; ++i)
    {
        auto textElement = new QCPTextElement(this, tr("Marker %1").arg(i + 1), QFont("sans", 8, QFont::Bold));
        textElement->setTextColor(MARKER_COLOR[i]);
        MarkElement->addElement(0, i, textElement);
    }
    MarkElement->setVisible(false);

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

    BandwidthSeries->setVisible(false);

    InitMenu();
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](QPoint pos) {
        if (MenuAppear)
            menu->popup(mapToGlobal(pos));
        MenuAppear = true;
    });

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event) {
        UpdateTrack(event);
        UpdateIndex(event);
        UpdateRuler(event);
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
        {
            TrackStartFreq = x;
            LeftButtonPress = true;
            break;
        }
        case FM_INDEX:
        {
            FM_StartFreq = x;
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
                break;
            }
            case TRACK:
            {
                TrackEndFreq = xAxis->pixelToCoord(event->pos().x());
                LeftButtonPress = false;
                break;
            }
            case FM_INDEX:
            {
                FM_EndFreq = xAxis->pixelToCoord(event->pos().x());
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
}

void ChartViewSpectrumWB::replace(unsigned char* const buf)
{
    if (!ready)
        return;
    ready = false;
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
    AnalyzeFM_Index(StartFreq, freq_step, DataPoint);
    emit triggerRefStatus(param->RefStatus);

    QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
    auto x = StartFreq;
    for (int i = 0; i < param->DataPoint; ++i)
    {
        amplx[i] = x;
        double y = (short)amplData[i] + AMPL_OFFSET;
        if (std::abs(x - 28) < freq_step)
            y += 1;
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
    replot(QCustomPlot::rpQueuedReplot);
}

void ChartViewSpectrumWB::SaveSpectrum(const QString& path)
{
    MarkElement->setVisible(true);
    replot(QCustomPlot::rpQueuedReplot);
    savePng(path);
    MarkElement->setVisible(false);
}

void ChartViewSpectrumWB::InitMenu()
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

    auto trackAction = new QAction(tr("Track"));
    menu->addAction(trackAction);
    connect(trackAction, &QAction::triggered, this, [this] {
        FM_IndexSeries->setVisible(false);
        TrackSeries->setVisible(true);
        DisplayState = TRACK;
    });

    auto measureAction = new QAction(tr("Measure"));
    menu->addAction(measureAction);
    connect(measureAction, &QAction::triggered, this, [this] {
        TrackSeries->setVisible(false);
        FM_IndexSeries->setVisible(true);
        DisplayState = FM_INDEX;
    });
}

void ChartViewSpectrumWB::AnalyzeFrame(long long DataPoint)
{
    if (pointsMax.size() != DataPoint)
    {
        pointsMax.resize(DataPoint);
        pointsMin.resize(DataPoint);
        for (auto i = 0ll; i < DataPoint; ++i)
        {
            pointsMax[i] = MIN_AMPL;
            pointsMin[i] = 0;
        }
    }
}

void ChartViewSpectrumWB::UpdateTrack(QMouseEvent *event)
{
    if (DisplayState == TRACK && LeftButtonPress)
    {
        auto xValue = xAxis->pixelToCoord(event->pos().x());
        QVector<double> x{ TrackStartFreq, TrackStartFreq, xValue, xValue }, y{ MAX_AMPL, MIN_AMPL, MIN_AMPL, MAX_AMPL };
        TrackSeries->setData(x, y, true);
    }
}

void ChartViewSpectrumWB::UpdateIndex(QMouseEvent *event)
{
    if (DisplayState == FM_INDEX && LeftButtonPress)
    {
        auto xValue = xAxis->pixelToCoord(event->pos().x());
        QVector<double> x{ FM_StartFreq, FM_StartFreq, xValue, xValue }, y{ MAX_AMPL, MIN_AMPL, MIN_AMPL, MAX_AMPL };
        FM_IndexSeries->setData(x, y, true);
    }
}

void ChartViewSpectrumWB::UpdateRuler(QMouseEvent *event)
{
    if (DisplayState == NORMAL && LeftButtonPress)
    {
        auto xValue = xAxis->pixelToCoord(event->pos().x());
        QVector<double> x{ xValue - 75e-3, xValue - 75e-3, xValue + 75e-3, xValue + 75e-3 }, y{ MAX_AMPL, MIN_AMPL, MIN_AMPL, MAX_AMPL };
        BandwidthSeries->setData(x, y, true);
    }
}

void ChartViewSpectrumWB::SeriesSelectChanged(bool MaxKeepSelect, bool MinKeepSelect)
{
    MaxKeepSeries->setVisible(this->MaxKeepSelect = MaxKeepSelect);
    MinKeepSeries->setVisible(this->MinKeepSelect = MinKeepSelect);
}

void ChartViewSpectrumWB::AnalyzeMark(double StartFreq, double freq_step, unsigned char* amplData, int DataPoint)
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


void ChartViewSpectrumWB::AnalyzeFM_Index(double StartFreq, double freq_step, int DataPoint)
{
    auto LargerFreq = std::max(FM_StartFreq, FM_EndFreq), SmallerFreq = std::min(FM_StartFreq, FM_EndFreq);
    auto indexStart = (LargerFreq - StartFreq) / freq_step;
    if (indexStart <= 0 || indexStart >= DataPoint)
        return;
    auto indexEnd = (SmallerFreq - StartFreq) / freq_step;
    if (indexEnd <= 0 || indexEnd >= DataPoint)
        return;
    emit triggerFM_Index(LargerFreq - SmallerFreq);
}

void ChartViewSpectrumWB::AnalyzeMeasure(double StartFreq, double freq_step, int DataPoint)
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

void ChartViewSpectrumWB::AnalyzeTrack(double StartFreq, double freq_step, unsigned char* amplData, int DataPoint)
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
    if (std::abs(StartFreq + freq_step * MaxFreqIndex - 28) < freq_step)
        MaxAmpl += 1;
    emit triggerTrack(StartFreq + freq_step * MaxFreqIndex, MaxAmpl);
}
