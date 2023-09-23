#ifndef THREADAUDIO_H
#define THREADAUDIO_H

#include <QThread>
#include <QAudio>
#include <QAudioSink>
#include <QBuffer>
#include "ThreadSafeQueue.h"

class ThreadAudio: public QThread
{
    Q_OBJECT
public:
    ThreadAudio(QObject * = nullptr);
    void execute(const std::shared_ptr<unsigned char[]>&);
    void Stop();
protected:
    void run() override;
private:
    QBuffer m_audioBuffer;
    QAudioSink* out = nullptr;
    threadsafe_queue<std::shared_ptr<unsigned char[]>> queue;
};

#endif // THREADAUDIO_H
