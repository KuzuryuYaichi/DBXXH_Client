#include "SampleRecv.h"
#include <thread>
#include <chrono>

SampleRecv::SampleRecv(TcpSocket* socket, QObject* parent): QObject(parent), m_socket(socket)
{
    tcpClient = new QTcpSocket;

    connect(this, &SampleRecv::SetParams, this, [this] {
        qDebug() << "Start Switch Freq: " << QString::number(freqNow / 1e3) + "MHz";
        auto freStr = ":FREQ " + QString::number(freqNow / 1e3) + "MHZ\r\n";
        tcpClient->write(freStr.toUtf8());

        std::thread([this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            m_socket->test_channel(1, freqNow * 1e3);
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            SampleSetFlag = true;
        }).detach();
    });
}

void SampleRecv::onP1dB(unsigned int CenterFreq)
{
    if (SampleSetFlag && CenterFreq == freqNow)
    {
        SampleSetFlag = false;
        if (freqNow < StopFreq)
        {
            SwitchFreq(CenterFreq + 1e3);
        }
    }
}

void SampleRecv::SwitchFreq(int freq)
{
    freqNow = freq;
    emit SetParams();
}

bool SampleRecv::startP1dB(int StartFreq, int StopFreq, QString ip, int port)
{
    this->StartFreq = StartFreq * 1e3;
    this->StopFreq = StopFreq * 1e3;
    tcpClient->connectToHost(ip, port);
    if (!tcpClient->waitForConnected(500))
    {
        return false;
    }
    freqNow = this->StartFreq;
    SampleSetFlag = true;
    tcpClient->write(":OUTPut ON\r\n");
    tcpClient->write(":Power 0dbm\r\n");
    SampleSetFlag = false;
    SwitchFreq(freqNow);
    return true;
}

void SampleRecv::stopP1dB()
{
    SampleSetFlag = false;
}
