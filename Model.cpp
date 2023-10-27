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
//    connect(this, &Model::sendDeviceStatus, m_mainWidget->statusEdit, &SideWidget::updateStatus);
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

    dataProcess = std::make_unique<DataProcess>(m_mainWidget->wbSignalDetectWidget);
    dataProcess->ProcessData();

    processThread = std::thread([this]
    {
        while (Running)
        {
            auto [res, data] = socket->spsc_queue.wait_and_pop();
            if (!res)
                continue;
            auto buf = data.get();
            switch (((DataHead*)buf)->PackType)
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
                break;
            }
            case 0x515:
            {
                dataProcess->execute(data);
                showDataWB(data);
                break;
            }
            case 0x602:
            {
                showDataNB(data);
                break;
            }
            case 0x605:
            {
                showDataPulse(data);
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

void Model::showDataWB(const std::shared_ptr<unsigned char[]>& data)
{
    auto param = (ParamPowerWB*)(data.get() + sizeof(DataHead));
    m_mainWidget->chartWB->replace(data);
    emit updatetime(param->Time);
}

void Model::showDataNB(const std::shared_ptr<unsigned char[]>& data)
{
    auto head = (DataHead*)data.get();
    m_channelWidget->replace(data, head->UnUsed);
}

void Model::showDataPulse(const std::shared_ptr<unsigned char[]>& data)
{
    auto pulse = (Pulse*)(data.get() + sizeof(DataHead));
    m_mainWidget->wbSignalDetectWidget->PulseDetect(pulse, 16);
}
