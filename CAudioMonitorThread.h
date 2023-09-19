#ifndef CAUDIOMONITORTHREAD_H
#define CAUDIOMONITORTHREAD_H

#include <QThread>
#include <QAudio>
#include <QAudioSink>
#include <QBuffer>
#include "ThreadSafeQueue.h"

class CAudioMonitorThread: public QThread
{
    Q_OBJECT
public:
    CAudioMonitorThread(QObject * = nullptr);
    void execute(const std::shared_ptr<unsigned char[]>&);
    void Stop();
protected:
    void run() override;
private:
    QBuffer m_audioBuffer;
    QAudioSink* out = nullptr;
    threadsafe_queue<std::shared_ptr<unsigned char[]>> queue;
};

#endif // CAUDIOMONITORTHREAD_H
