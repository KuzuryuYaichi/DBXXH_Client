#include "Model.h"

#include <QStatusBar>
#include "StructNetData.h"
#include "global.h"

Model::Model(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle(tr("Client"));
    socket = std::make_shared<TcpSocket>();
    setCentralWidget(m_tabWidget = new QTabWidget);
    m_tabWidget->addTab(m_channelWidget = new ChannelWidget(socket.get()), tr("NB"));
    m_tabWidget->insertTab(0, m_mainWidget = new MainWidget(socket.get(), m_channelWidget->chartNB[0]), tr("WB"));
    m_tabWidget->setCurrentIndex(0);

    statusTimer = new QTimer;
    statusTimer->setSingleShot(true);
    connect(this, &Model::sendDeviceStatus, m_mainWidget->statusEdit, &SideWidget::updateStatus);
    setStatusBar(statusBar = new QStatusBar(this));
    connect(this, &Model::updatetime, this, [this](unsigned long long timeData) {
        if (!readyTime)
            return;
        readyTime = false;
        statusBar->showMessage(timeConvert(timeData).toString(DATETIME_FORMAT));
        statusTimer->start(500);
    });
    connect(statusTimer, &QTimer::timeout, this, [this] {
        readyTime = true;
    });

    dataProcess = std::make_unique<DataProcess>(m_mainWidget->wBSignalDetectWidget);
    dataProcess->ProcessData();

    processThread = std::thread([this]
    {
        while (Running)
        {
            auto [res, packet] = socket->spsc_queue.wait_and_pop();
            if (!res)
                continue;
            auto buf = packet.get();
            auto head = (DataHead*)buf;
            switch (head->PackType)
            {
            case 0x511:
            {
                auto parameter = (StructWorkCommandRev*)(buf + sizeof(DataHead));
                static QString LabelText = tr("Work Params:\n Resolution:%1 kHz\n SimBand:20 MHz\n Gain Mode:%2\n");
                m_statusList.append(LabelText.arg(parameter->FreqRes).arg(parameter->GainMode? tr("AGC"): tr("MGC")));
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
                dataProcess->execute(packet);
                showDataWB(buf);
                break;
            }
            case 0x602:
            {
                showDataNB(buf, QDateTime());
                break;
            }
            }
        }
    });
}

Model::~Model()
{
    Running = false;
    socket->spsc_queue.clean();
    if (processThread.joinable())
        processThread.join();
}

long long FileTimeToMillSeconds(unsigned long long pTime)
{
    return (long long)(pTime - 116444736000000000) / 10000;
}

QDateTime Model::timeConvert(unsigned long long time)
{
    return QDateTime::fromMSecsSinceEpoch(FileTimeToMillSeconds(time));
}

void Model::showDataWB(unsigned char* const buf)
{
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x515:
    {
        auto param = (ParamPowerWB*)(buf + sizeof(DataHead));
        m_mainWidget->chartWB->replace(buf);
        emit updatetime(param->Time);
        break;
    }
    }
}

void Model::showDataNB(unsigned char* const buf, const QDateTime&)
{
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x602:
    {
        m_channelWidget->replace(buf, head->UnUsed);
        break;
    }
    }
}
