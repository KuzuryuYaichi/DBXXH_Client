#include "Model.h"

#include <QStatusBar>
#include "StructNetData.h"
#include "global.h"

Model::Model(QWidget *parent): QMainWindow(parent), spsc_queue(isRunning)
{
    setWindowTitle(tr("Client"));
    m_socket = new TcpSocket(spsc_queue);
    setCentralWidget(m_tabWidget = new QTabWidget);
    m_tabWidget->addTab(m_cxWidget = new ChartWidget(m_socket), tr("宽带"));
    m_tabWidget->addTab(m_zcWidget = new ZCWidget(m_socket), tr("窄带"));

    connect(this, &Model::sendDeviceStatus, m_cxWidget->statusEdit, &SideWidget::updateStatus);
    setStatusBar(statusBar = new QStatusBar(this));
    connect(this, &Model::updatetime, this, [this](QString str) {
        statusBar->showMessage(str);
    });

    pool = std::make_shared<FixedThreadPool>(m_cxWidget);
    pool->Start();

    processThread = std::thread([this]
    {
        while (isRunning)
        {
            auto packet = spsc_queue.wait_and_pop();
            if (packet == nullptr)
                continue;
            auto buf = packet.get();
            auto head = (DataHead*)buf;
            switch (head->PackType)
            {
            case 0x511:
            {
                auto parameter = (StructWorkCommandRev*)(buf + sizeof(DataHead));
                static QString rcvMode;
                switch (parameter->RcvMode)
                {
                case 0:
                {
                    rcvMode = tr("Normal");
                    break;
                }
                case 1:
                {
                    rcvMode = tr("Low Distortion");
                    break;
                }
                case 2:
                {
                    rcvMode = tr("Low Noise");
                    break;
                }
                }
                static QString LabelText = tr("Work Params:\n Resolution:%1 kHz\n SimBand:20 MHz\n Gain Mode:MGC\n Gain:%2 dB\n Receiver Work Mode:%3");
                m_statusList.append(LabelText.arg(parameter->FreqRes).arg(parameter->MGC).arg(rcvMode));
                emit sendDeviceStatus(m_statusList);
                m_statusList.clear();
                break;
            }
            case 0x500:
            {
                auto control = (StructControlRev*)(buf + sizeof(DataHead));
                switch (control->ControlFlag)
                {
                case 0:
                {
                    m_statusList.append(tr("Control Failed"));
                    emit sendDeviceStatus(m_statusList);
                    break;
                }
                case 1:
                {
                    m_statusList.append(tr("Control Succeed"));
                    emit sendDeviceStatus(m_statusList);
                    break;
                }
                }
                m_statusList.clear();
                break;
            }
            case 0x512:
            {
                auto self_check = (StructDeviceScheckRev*)(buf + sizeof(DataHead));
                QStringList statusList;
                for (int i = 0; i < CHANNEL_NUM; ++i)
                {
                    statusList.append(self_check->ScheckResult & (1 << i)? tr("Fault"): tr("OK"));
                }
                statusList.append(self_check->ScheckResult & (1 << 30)? tr("Fault"): tr("OK"));
                statusList.append(self_check->ScheckResult & (1 << 31)? tr("Fault"): tr("OK"));
                for (int i = 0; i < CHANNEL_NUM; ++i)
                {
                    statusList.append(self_check->AScheckResult & (1 << i)? tr("Fault"): tr("OK"));
                }
                static QString LabelText = tr("\nReceiverChannel:%1\nChannel1:%2, Channel2:%3, Channel3:%4, Channel4:%5\nLO Status:%6\nCorrectStatus:%7\nAntennas:%8\nAntenna1:%9, Antenna2:%10, Antenna3:%11, Antenna4:%12");
                m_statusList.append(LabelText.arg(self_check->DeviveChNum)
                        .arg(statusList[0]).arg(statusList[1]).arg(statusList[2]).arg(statusList[3])
                        .arg(statusList[4]).arg(statusList[5]).arg(self_check->AGroupNum)
                        .arg(statusList[6]).arg(statusList[7]).arg(statusList[8]).arg(statusList[9]));
                emit sendDeviceStatus(m_statusList);
                m_statusList.clear();
                break;
            }
            case 0x515:
            {
//                auto param = (StructFixedCXResult*)(buf + sizeof(DataHead));
//                const auto GROUP_LENGTH = sizeof(long long) + (sizeof(char) + sizeof(short)) * param->DataPoint;
//                auto data = buf + sizeof(DataHead) + sizeof(StructFixedCXResult);
//                int milliseconds = 0;
//                auto timeFormat = timeConvert(*(long long*)data);
//                m_cxWidget->TestingTime(timeFormat, milliseconds);

                static std::vector<std::shared_ptr<char[]>> packets;
                static int index = 0;
                if (index == 0)
                {
                    packets = std::vector<std::shared_ptr<char[]>>(JUDGE_ARRAY_DEPTH);
                }
                packets[index] = packet;
                if (++index == JUDGE_ARRAY_DEPTH)
                {
                    index = 0;
                    pool->execute(std::move(packets));
                }
                showDataCX(buf);
                break;
            }
            case 0x513:
            {
//                auto param = (StructSweepCXResult*)(buf + sizeof(DataHead));
                showDataCX(buf);
                break;
            }
            case 0x516:
            {
                showDataCX(buf);
                break;
            }
            case 0x602:
            {
//                auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
//                int milliseconds = 0;
//                auto timeFormat = timeConvert(param->StartTime);
                showDataZC(buf, QDateTime());
                break;
            }
            }
        }
    });
}

Model::~Model()
{
    spsc_queue.clean();
    if (processThread.joinable())
        processThread.join();
}

long long FileTimeToMillSeconds(unsigned long long pTime)
{
    return (long long)(pTime - 116444736000000000) / 10000;
}

QDateTime Model::timeConvert(unsigned long long time)
{
    return QDateTime::fromMSecsSinceEpoch(FileTimeToMillSeconds(time)).addSecs(1);
}

void Model::showDataCX(char* const buf)
{
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x515:
    {
        auto param = (StructFixedCXResult*)(buf + sizeof(DataHead));
        const auto GROUP_LENGTH = sizeof(long long) + (sizeof(char) + sizeof(short)) * param->DataPoint;
        auto data = buf + sizeof(DataHead) + sizeof(StructFixedCXResult);
        auto freq_step = param->FreqResolution / 1e3;
        auto start_freq = param->CenterFreq / 1e3 - BAND_WIDTH_MHZ / 2;
        auto timeFormat = timeConvert(*(unsigned long long*)data);
        for (int g = 0; g < 1; ++g)
        {
            auto amplData = (unsigned char*)(data + sizeof(long long));
            auto doaData = (short*)(amplData + param->DataPoint);
            auto x = start_freq;
            QVector<double> amplx(param->DataPoint), amply(param->DataPoint);
            QVector<QPointF> doa;
            for (int i = 0; i < param->DataPoint; ++i)
            {
                auto range = (short)amplData[i] + AMPL_OFFSET;
                amplx[i] = x; amply[i] = range;
                if (range > m_cxWidget->threshold)
                {
                    auto direction = doaData[i];
                    if (direction < UPPER_DIRECTION && direction > LOWER_DIRECTION)
                    {
                        doa.append(QPointF(x, direction / 10.0));
                    }
                }
                x += freq_step;
            }
            if (g == 0)
            {
                emit updatetime(timeFormat.toString("yyyy/MM/dd hh:mm:ss"));
            }
            data += GROUP_LENGTH;
        }
        break;
    }
    case 0x513:
    {
        auto param = (StructSweepCXResult*)(buf + sizeof(DataHead));
        auto timeStruct = (StructSweepTimeData*)(buf + sizeof(DataHead) + sizeof(StructSweepCXResult));
        auto dataRangeDirection = (StructSweepRangeDirectionData*)(timeStruct + param->TimeNum);
        auto freq_step = param->FreqResolution / 1e3, start_freq = param->StartFreq / 1e3 - BAND_WIDTH_MHZ / 2, x = start_freq;
        QVector<double> amplx(param->CXResultPoint), amply(param->CXResultPoint);
        QVector<QPointF> doa;
        auto timeFormat = timeConvert((unsigned long long)param->StartTime);
        for (int i = 0; i < param->CXResultPoint; ++i)
        {
            auto range = (short)dataRangeDirection[i].Range + AMPL_OFFSET;
            amplx[i] = x; amply[i] = range;
            if (range > m_cxWidget->threshold)
            {
                auto direction = dataRangeDirection[i].Direction / 10.0;
                if (direction < UPPER_DIRECTION && direction > LOWER_DIRECTION)
                {
                    doa.append(QPointF(x, direction));
                }
            }
            x += freq_step;
        }
        emit updatetime(timeFormat.toString("yyyy/MM/dd hh:mm:ss"));
        break;
    }
    }
}

void Model::showDataZC(char* const buf, const QDateTime&)
{
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x0602:
    {
        auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
        if (param->Accuracy != 1 || param->ChannelNum != 2)
            return;
        m_zcWidget->FFT_Trans(param, head->UnUsed);
        break;
    }
    }
}
