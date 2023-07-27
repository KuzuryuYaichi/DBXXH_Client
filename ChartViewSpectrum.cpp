#include "ChartViewSpectrum.h"
#include "StructNetData.h"

ChartViewSpectrum::ChartViewSpectrum(QString title, QString X_title, int AXISX_MIN, int AXISX_MAX, QString Y_title, int AXISY_MIN, int AXISY_MAX, QWidget* parent):
    ChartViewCustom(title, X_title, Y_title, parent)
{
    xAxis->setRange(AXISX_MIN, AXISX_MAX);
    yAxis->setRange(AXISY_MIN, AXISY_MAX);

    addGraph();
    graph(0)->setPen(QPen(Qt::blue));
    graph(0)->setLineStyle(QCPGraph::lsLine);
    graph(0)->rescaleAxes(true);

    addGraph();
    graph(1)->setPen(QPen(Qt::red));
    graph(1)->setLineStyle(QCPGraph::lsLine);
    graph(1)->rescaleAxes(true);
    QVector<double> x {MIN_FREQ, MAX_FREQ}, y {0, 0};
    graph(1)->setData(x, y);

    tracer = new QCPItemTracer(this);
    tracer->setPen(QPen(Qt::red));
    tracer->setBrush(QBrush(Qt::red));
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setSize(5);

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent *event) {
        UpdateRuler(event);
        UpdateTracer(event);
        replot(QCustomPlot::rpQueuedReplot);
    });

    connect(this, &QCustomPlot::mousePress, this, [this](QMouseEvent *event) {
        if (event->button() == Qt::LeftButton)
        {
            auto x = xAxis->pixelToCoord(event->pos().x());
            auto y = yAxis->pixelToCoord(event->pos().y());
            if (xAxis->range().contains(x) && yAxis->range().contains(y))
                isPress = true;
        }
        replot(QCustomPlot::rpQueuedReplot);
    });

    connect(this, &QCustomPlot::mouseRelease, this, [this](QMouseEvent *event) {
        if (event->button() == Qt::LeftButton)
        {
            isPress = false;
            emit thresholdEnterPressedSignal(graph(1)->data().data()->at(0)->value);
        }
        replot(QCustomPlot::rpQueuedReplot);
    });

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](QPoint pos) {
        auto menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        auto x = xAxis->pixelToCoord(pos.x());
        auto action = new QAction((QString::number(x) + "MHz"), this);
        action->setEnabled(false);
        menu->addAction(action);
        menu->popup(mapToGlobal(pos));
        for (int i = 0; i < 3; ++i)
        {
            auto action = new QAction(tr("Add To Marker%1").arg(i + 1), this);
            menu->addAction(action);
            connect(action, &QAction::triggered, this, [this, i, x] {

            });
        }
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
    QToolTip::showText(tracer->position->pixelPosition().toPoint(), QString("%1MHz, %2dBm").arg(xValue).arg(yValue));
}

void ChartViewSpectrum::replace(char* const buf)
{
    if (!ready)
        return;
    ready = false;
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x515:
    {
        auto param = (StructFixedCXResult*)(buf + sizeof(DataHead));
        auto DataPoint = param->DataPoint;
        const auto GROUP_LENGTH = sizeof(long long) + (sizeof(char) + sizeof(short)) * DataPoint;
        auto data = buf + sizeof(DataHead) + sizeof(StructFixedCXResult);
        auto freq_step = param->FreqResolution / 1e3;
        auto start_freq = param->CenterFreq / 1e3 - BAND_WIDTH_MHZ / 2;
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
    case 0x513:
    {
        auto param = (StructSweepCXResult*)(buf + sizeof(DataHead));
        auto DataPoint = param->CXResultPoint;
        auto timeStruct = (StructSweepTimeData*)(buf + sizeof(DataHead) + sizeof(StructSweepCXResult));
        auto dataRangeDirection = (StructSweepRangeDirectionData*)(timeStruct + param->TimeNum);
        QVector<double> amplx(DataPoint), amply(DataPoint);
        auto freq_step = param->FreqResolution / 1e3, start_freq = param->StartFreq / 1e3 - BAND_WIDTH_MHZ / 2, x = start_freq;
        for (int i = 0; i < param->CXResultPoint; ++i)
        {
            auto range = (short)dataRangeDirection[i].Range + AMPL_OFFSET;
            amplx[i] = x; amply[i] = range;
            x += freq_step;
        }
        graph(0)->setData(amplx, amply, true);
        break;
    }
    default: return;
    }
    replot(QCustomPlot::rpQueuedReplot);
}
