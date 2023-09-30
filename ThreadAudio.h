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
    ~ThreadAudio();
    void execute(const std::shared_ptr<unsigned char[]>&);
public slots:
    void stateChanged(QAudio::State);
protected:
    void run() override;
private:
    static constexpr std::pair<int, int> SAMPLE_RATE[] = { { 150, 375 },
                                              { 300, 750 },
                                              { 600, 1500 },
                                              { 1500, 3750 },
                                              { 2400, 6000 },
                                              { 6000, 15000 },
                                              { 9000, 22500 },
                                              { 15000, 37500 },
                                              { 30000, 75000 },
                                              { 50000, 125000 },
                                              { 120000, 300000 },
                                              { 150000, 375000 } };
    std::unique_ptr<QAudioSink> AudioSink;
    threadsafe_queue<std::shared_ptr<unsigned char[]>> queue;
    void ParamChanged(int);
    QIODevice* io = nullptr;
};

#endif // THREADAUDIO_H
