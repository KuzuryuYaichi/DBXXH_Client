#include "ThreadAudio.h"

#include <QUrl>
#include <QBuffer>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QDebug>
#include <QMediaDevices>

#include "global.h"
#include "StructNetData.h"

ThreadAudio::ThreadAudio(QObject *parent): QThread(parent)
{
    QAudioFormat fmt;
    fmt.setSampleRate(375000);
    fmt.setChannelCount(1);
    fmt.setSampleFormat(QAudioFormat::Int16);
    out = new QAudioSink(QMediaDevices::defaultAudioOutput(), fmt, this);
    connect(out, &QAudioSink::stateChanged, this, [](QAudio::State newState) {
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

void ThreadAudio::Stop()
{
    queue.clean();
}

void ThreadAudio::execute(const std::shared_ptr<unsigned char[]>& task)
{
    queue.push(task);
}

void ThreadAudio::run()
{
    auto io = out->start();
    io->open(QIODevice::ReadWrite);
    auto size = out->bufferSize();
    auto tmp = new char[size];
    auto buffer = std::shared_ptr<char[]>(tmp);
    while (true)
    {
        auto [res, d] = queue.wait_and_pop();
        if (!res)
            return;
        auto buf = d.get();
        auto head = (DataHead*)buf;
        if (head->PackType != 0x602)
            return;
        auto param = (StructNBWaveZCResult*)(buf + sizeof(DataHead));
        auto data = (NarrowDDC*)(param + 1);
        auto SampleRate = 375000;
        switch (param->BandWidth)
        {
        case 150: SampleRate = 375; break;
        case 300: SampleRate = 750; break;
        case 600: SampleRate = 1500; break;
        case 1500: SampleRate = 3750; break;
        case 2400: SampleRate = 6000; break;
        case 6000: SampleRate = 15000; break;
        case 9000: SampleRate = 22500; break;
        case 15000: SampleRate = 37500; break;
        case 30000: SampleRate = 75000; break;
        case 50000: SampleRate = 125000; break;
        case 120000: SampleRate = 300000; break;
        case 150000: SampleRate = 375000; break;
        }

        m_audioBuffer.open(QIODevice::ReadWrite);
        for (auto i = 0; i < DDC_LEN; ++i)
            m_audioBuffer.write((char*)&data[i].I, sizeof(short));
        m_audioBuffer.seek(0);
        while (!m_audioBuffer.atEnd())
        {
            std::memset(buffer.get(), 0, size);
            if (out->bytesFree() == 0)
            {
                continue;
            }
            if (m_audioBuffer.read(buffer.get(), size) <= 0)
            {
                break;
            }
            io->write(buffer.get(), size);
        }
        m_audioBuffer.close();
    }
    io->close();
}
