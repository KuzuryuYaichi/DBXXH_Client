#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QTcpSocket>
#include <QTimer>
#include "ThreadSafeQueue.h"

class TcpSocket: public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpSocket(threadsafe_queue<std::shared_ptr<char[]>>&, QObject* = nullptr);
    void self_check(const uint);
    void work_ctrl(const uint);
    void parameter_set();
    void broad_band(const uint, uint);
    void narrow_band(const uint, uint);
    void sweep(const uint, uint, uint);
    void test_channel(const uint, uint);
    void nb_receiver(const uint, const uint);
    void nb_channel(const uint, const uint, const uint, const uint);

signals:
    void sendSocketStatus(QString status);
    void sendTcpMessage(QByteArray ba);

public slots:
    void connectToServer(const QString& addr, const unsigned short& port);

private:
    unsigned int task_id = 0;
    threadsafe_queue<std::shared_ptr<char[]>>& spsc_queue;
    QTimer* m_timer;
    QString m_addr;
    quint16 m_port;

    static constexpr int SOCK_OPT = 1024 * 1024 * 3;
    static constexpr short CTRL_TYPE = 0x08FE;
};

#endif // TCPSOCKET_H
