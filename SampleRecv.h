#include "global.h"
#include "StructSample.h"
#include "TcpSocket.h"

class SampleRecv: public QObject
{
    Q_OBJECT
public:
    SampleRecv(TcpSocket*, QObject* = nullptr);

    TcpSocket* m_socket;
    QTcpSocket* tcpClient;
    int freqNow;
    bool SampleSetFlag = false;
    StructSample sample;

    struct P1dBParam
    {
        float freq;
        float power;
        float powerOffSet;
        P1dBParam(float freq, float power, float powerOffSet): freq(freq), power(power), powerOffSet(powerOffSet) {}
    };

signals:
    void SetParams();

public:
    void onP1dB(unsigned int CenterFreq);
    void SwitchFreq(int);
    bool startP1dB(int, int, QString, int);
    void stopP1dB();

private:
    int StartFreq = MIN_SAMPLE_FREQ;
    int StopFreq = MAX_SAMPLE_FREQ;
};
