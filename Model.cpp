#include "Model.h"

#include <QStatusBar>
#include "StructNetData.h"
#include "global.h"

Model::Model(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle(tr("Client"));
    m_socket = std::make_shared<TcpSocket>();
    setCentralWidget(m_tabWidget = new QTabWidget);
    m_tabWidget->addTab(m_cxWidget = new ChartWidget(m_socket.get()), tr("宽带"));
    m_tabWidget->addTab(m_zcWidget = new ZCWidget(m_socket.get()), tr("窄带"));

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
            auto packet = m_socket->spsc_queue.wait_and_pop();
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
            case 0x503:
            {
                showDataCX(buf);
                break;
            }
            case 0x602:
            {
                showDataZC(buf, QDateTime());
                break;
            }
            }
        }
    });
}

Model::~Model()
{
    isRunning = false;
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

void Model::showDataCX(unsigned char* const buf)
{
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x503:
    {
        auto param = (ParamPowerWB*)(buf + sizeof(DataHead));
        m_cxWidget->chartWB->replace(buf);
        emit updatetime(timeConvert(param->Time).toString("yyyy/MM/dd hh:mm:ss"));
        break;
    }
    }
}

void Model::showDataZC(unsigned char* const buf, const QDateTime&)
{
    auto head = (DataHead*)buf;
    switch (head->PackType)
    {
    case 0x0602:
    {
        m_zcWidget->replace(buf, head->UnUsed);
        break;
    }
    }
}
