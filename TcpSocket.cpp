#include "TcpSocket.h"

#include "global.h"
#include "ThreadSafeQueue.h"
#include <QHostAddress>
#include "StructNetData.h"
#include "global.h"

extern PARAMETER_SET g_parameter_set;

TcpSocket::TcpSocket(threadsafe_queue<std::shared_ptr<char[]>>& spsc_queue, QObject *parent): QTcpSocket(parent), spsc_queue(spsc_queue)
{
    setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, SOCK_OPT);
    setReadBufferSize(SOCK_OPT);
    connect(this, &QTcpSocket::readyRead, this, [this] {
        while (bytesAvailable())
        {
            if (bytesAvailable() < sizeof(DataHead))
                return;
            auto peek_data = peek(sizeof(DataHead));
            auto head = (DataHead*)peek_data.data();
            auto block_size = head->PackLen;
            if (bytesAvailable() < block_size)
                return;
//            auto data = std::make_shared<char[]>(block_size);
            auto tmp = new char[block_size];
            auto data = std::shared_ptr<char[]>(tmp);
            unsigned int count = 0;
            while (count < block_size)
            {
                auto nbytes = read(data.get() + count, block_size - count);
                if (nbytes <= 0)
                    return;
                count += nbytes;
            }
            this->spsc_queue.push(data);
        }
    });

    connect(this, &TcpSocket::sendTcpMessage, this, [this](QByteArray ba) {
        write(ba.data(), ba.length());
        ++task_id;
    });

    connect(this, &QTcpSocket::disconnected, this, [this]
    {
        emit sendSocketStatus(tr("Server Disconnected"));
        m_timer = new QTimer();
        m_timer->setInterval(3000);
        m_timer->setSingleShot(false);
        connect(m_timer, &QTimer::timeout, this, [this] {
            connectToHost(QHostAddress(m_addr), m_port);
            if(!waitForConnected(200)) {
                emit sendSocketStatus(tr("Try to Reconnect To Server..."));
            } else {
                m_timer->stop();
                m_timer->deleteLater();
                emit sendSocketStatus(tr("Connected To Server"));
            }
        });
        m_timer->start();
    });
}

void TcpSocket::self_check(const uint mode)
{
    char tmp[100];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0201;SCheck:%d\r\n", task_id, mode);
    QString ss(tmp);
    QByteArray text;
    DataHead head(CTRL_TYPE, ss.size() + sizeof(DataHead));
    text.append((char*)&head, sizeof(DataHead));
    text.append(ss.toLocal8Bit());
    emit sendTcpMessage(text);
}

void TcpSocket::work_ctrl(const uint mode)
{
    char tmp[100];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0202;WorkCtrl:%d\r\n", task_id, mode);
    QString ss(tmp);
    QByteArray text;
    DataHead head(CTRL_TYPE, ss.size() + sizeof(DataHead));
    text.append((char*)&head, sizeof(DataHead));
    text.append(ss.toLocal8Bit());
    emit sendTcpMessage(text);
}

void TcpSocket::parameter_set()
{
    char tmp[200];
    std::sprintf(tmp, "Task:%ud;Sta:2;Type:0203;Data:%ud;Detect:%ud;FreqRes:%f;SimBW:%ud;GMode:%u;MGC:%d;AGC:%d;SmNum:%ud;SmMode:%ud;LmMode:%ud;LmVal:%ud;RcvMode:%ud\r\n",
            task_id, g_parameter_set.Data, g_parameter_set.Detect, g_parameter_set.FreqRes, g_parameter_set.SimBW, g_parameter_set.GMode, g_parameter_set.MGC,
            g_parameter_set.AGC, g_parameter_set.SmNum, g_parameter_set.SmMode, g_parameter_set.LmMode, g_parameter_set.LmVal, g_parameter_set.RcvMode);
    QString ss(tmp);
    QByteArray text;
    DataHead head(CTRL_TYPE, ss.size() + sizeof(DataHead));
    text.append((char*)&head, sizeof(DataHead));
    text.append(ss.toLocal8Bit());
    emit sendTcpMessage(text);
}

void TcpSocket::broad_band(const uint act, uint freq_Hz)
{
    char tmp[100];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0206;Act:%d;CFreq:%d;CTime:10\r\n", task_id, act, freq_Hz);
    QString ss(tmp);
    QByteArray text;
    DataHead head(CTRL_TYPE, ss.size() + sizeof(DataHead));
    text.append((char*)&head, sizeof(DataHead));
    text.append(ss.toLocal8Bit());
    emit sendTcpMessage(text);
}

void TcpSocket::narrow_band(const uint act, uint freq_Hz)
{
    char tmp[100];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0205;Act:%d;DFreq:%d\r\n", task_id, act, freq_Hz);
    QString ss(tmp);
    QByteArray text;
    DataHead head(CTRL_TYPE, ss.size() + sizeof(DataHead));
    text.append((char*)&head, sizeof(DataHead));
    text.append(ss.toLocal8Bit());
    emit sendTcpMessage(text);
}

void TcpSocket::sweep(const uint act, uint sFreq_Hz, uint eFreq_Hz)
{
    char tmp[100];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0204;Act:%d;SFreq:%d;EFreq:%d\r\n", task_id, act, sFreq_Hz, eFreq_Hz);
    QString ss(tmp);
    QByteArray text;
    DataHead head(CTRL_TYPE, ss.size() + sizeof(DataHead));
    text.append((char*)&head, sizeof(DataHead));
    text.append(ss.toLocal8Bit());
    emit sendTcpMessage(text);
}

void TcpSocket::test_channel(const uint act, const uint freq_Hz)
{
    char tmp[100];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0207;Act:%d;CFreq:%d;Mode:0;Scope:60\r\n", task_id, act, freq_Hz);
    QString ss(tmp);
    QByteArray text;
    DataHead head(CTRL_TYPE, ss.size() + sizeof(DataHead));
    text.append((char*)&head, sizeof(DataHead));
    text.append(ss.toLocal8Bit());
    emit sendTcpMessage(text);
}

void TcpSocket::nb_receiver(const uint receiver, const uint freq_Hz)
{
    char tmp[200];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0403;RcvNum:%d;Freq:%d;Mod:1;BW:200;GMode:1;MGC:0;SQU:0;ATT:0;Vol:0;AFC:0;RcvMode:0\r\n",
                 task_id, receiver, freq_Hz);
    QString ss(tmp);
    QByteArray text;
    DataHead head(CTRL_TYPE, ss.size() + sizeof(DataHead));
    text.append((char*)&head, sizeof(DataHead));
    text.append(ss.toLocal8Bit());
    emit sendTcpMessage(text);
}

void TcpSocket::nb_channel(const uint receiver, const uint channel, const uint freq_Hz, const uint bandwidth)
{
    char tmp[200];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0411;RcvNum:%d;BankNum:%d;GMode:0;MGC:30;KpTime:2;Sn:1;Freq:%d;DDCBW:%d;DemodType:1;DemodRate:2000;\r\n",
                 task_id, receiver, channel, freq_Hz, bandwidth);
    QString ss(tmp);
    QByteArray text;
    DataHead head(CTRL_TYPE, ss.size() + sizeof(DataHead));
    text.append((char*)&head, sizeof(DataHead));
    text.append(ss.toLocal8Bit());
    emit sendTcpMessage(text);
}

void TcpSocket::connectToServer(const QString& addr, const unsigned short& port)
{
    m_addr = addr;
    m_port = port;
    connectToHost(QHostAddress(addr), port);
    if (!waitForConnected(200))
        emit sendSocketStatus(tr("Failed to Connect To Server"));
    else
        emit sendSocketStatus(tr("Connected To Server"));
}
