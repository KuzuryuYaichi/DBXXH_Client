#ifndef CAUDIOMONITORTHREAD_H
#define CAUDIOMONITORTHREAD_H

#include <QThread>
#include <QAudio>
#include <QAudioSink>
#include <QBuffer>

class CAudioMonitorThread : public QThread
{
    Q_OBJECT
public:
    CAudioMonitorThread(QObject *parent = nullptr);
    ~CAudioMonitorThread();
    void Stop();
protected:
    void run() override;
private slots:
    void OnstateChanged(QAudio::State);
private:
    bool m_bStop = false;
    std::shared_ptr<QBuffer> m_audioBuffer = nullptr;
    std::shared_ptr<QByteArray> m_pByte = nullptr;
    std::shared_ptr<QAudioSink> out = nullptr;
};

#endif // CAUDIOMONITORTHREAD_H
