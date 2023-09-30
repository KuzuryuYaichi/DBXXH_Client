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
    ParamChanged(275000);
}

ThreadAudio::~ThreadAudio()
{
    queue.clean();
}

void ThreadAudio::execute(const std::shared_ptr<unsigned char[]>& task)
{
    queue.push(task);
}

void ThreadAudio::ParamChanged(int SampleRate)
{
    QAudioFormat fmt;
    fmt.setChannelCount(1);
    fmt.setSampleFormat(QAudioFormat::Int16);
    fmt.setSampleRate(SampleRate);
    fmt.setChannelConfig(QAudioFormat::ChannelConfigMono);
    if (io)
        io->close();
    AudioSink = std::make_unique<QAudioSink>(QMediaDevices::defaultAudioOutput(), fmt);
    connect(AudioSink.get(), &QAudioSink::stateChanged, this, &ThreadAudio::stateChanged);
    io = AudioSink->start();
    io->open(QIODevice::ReadWrite);
}

void ThreadAudio::stateChanged(QAudio::State newState)
{
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
}

void ThreadAudio::run()
{
    while (true)
    {
        auto [res, pack] = queue.wait_and_pop();
        if (!res)
            return;
        auto param = (StructNBWaveZCResult*)(pack.get() + sizeof(DataHead));
        auto data = (NarrowDDC*)(param + 1);
        for (auto& [bound, sampleRate]: SAMPLE_RATE)
        {
            if (param->Bound == bound && AudioSink->format().sampleRate() != sampleRate)
            {
                ParamChanged(sampleRate);
                break;
            }
        }
        auto buffer = std::make_unique<short[]>(param->DataPoint);
        for (auto i = 0; i < param->DataPoint; ++i)
            buffer[i] = data[i].I;
        int offset = 0, length = param->DataPoint;
        while (offset < param->DataPoint)
        {
            if (AudioSink->bytesFree() == 0)
                continue;
            auto len = io->write((char*)buffer.get() + offset, length);
            if (len > 0)
            {
                offset += len;
                length -= len;
            }
        }
    }
    io->close();
}
