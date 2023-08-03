#include "CAudioMonitorThread.h"

#include <QUrl>
#include <QBuffer>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QDebug>

CAudioMonitorThread::CAudioMonitorThread(QObject *parent): QThread(parent)
{
    m_pByte = std::make_shared<QByteArray>();
    m_audioBuffer = std::make_shared<QBuffer>(m_pByte.get());

    QAudioFormat fmt; //通过fmt设定音频数据格式。只有明确知道音频数据的声道数、采样率、采样位数，才可以正常地播放
    fmt.setSampleRate(44100); //设定播放采样频率为44100Hz的音频文件
    fmt.setChannelCount(1); //设定播放声道数目为2通道（立体声）的音频文件。mono(平声道)的声道数目是1，stero(立体声)的声道数目是2
    fmt.setSampleFormat(QAudioFormat::Int16); //设定采样类型。根据采样位数来设定。采样位数为8或16位则设置为QAudioFormat::UnSignedInt

    QAudioDevice info;
    if (!info.isFormatSupported(fmt))
    {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return;
    }

    out = std::make_shared<QAudioSink>(fmt, this);
    connect(out.get(), &QAudioSink::stateChanged, this, [](QAudio::State newState) {
        switch (newState)
        {
        case QAudio::IdleState:
            qDebug() << "QAudio::IdleState";
            break;
        case QAudio::StoppedState:
            qDebug() << "QAudio::StoppedState";
            break;
        case QAudio::SuspendedState:
            qDebug() << "QAudio::SuspendedState";
            break;
        case QAudio::ActiveState:
            qDebug() << "QAudio::ActiveState";
            break;
        default:
            break;
        }
    });
}

CAudioMonitorThread::~CAudioMonitorThread()
{
}

void CAudioMonitorThread::Stop()
{
    m_bStop = true;
}

void CAudioMonitorThread::run()
{
    auto io = out->start();
    io->open(QIODevice::ReadWrite);
    auto size = out->bufferSize();
    auto tmp = new char[size];
    auto buffer = std::shared_ptr<char[]>(tmp);
    while (!m_bStop)
    {
        m_audioBuffer->open(QIODevice::ReadWrite);
//        m_audioBuffer->write(byt);
        m_audioBuffer->seek(0);

        while (!m_audioBuffer->atEnd())
        {
            std::memset(buffer.get(), 0, size);
            if (out->bytesFree() == 0)//声卡缓冲区无空闲时不写数据，跳过
            {
                continue;
            }
            if (m_audioBuffer->read(buffer.get(), size) <= 0)  //从缓冲据读取数据，如果读取到文件末尾或者读取不成功则通过break函数跳出while循环
            {
                break;
            }
            io->write(buffer.get(), size); //调用write函数将内存buf中的PCM数据写入到扬声器,即把buf中的数据提交到声卡发声
        }
        m_audioBuffer->close();
    }
    io->close();
}

void CAudioMonitorThread::OnstateChanged(QAudio::State)
{

}