#ifndef TCP_SESSION_H
#define TCP_SESSION_H

#include <memory>
#include <thread>
#include <unordered_set>
#include <mutex>
#include <string>

#include "StructNetData.h"
#include "ThreadSafeQueue.h"
#include "boost/asio.hpp"

class TcpSocket
{
public:
    TcpSocket();
    ~TcpSocket();
    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;
    void self_check(const unsigned int);
    void work_ctrl(const unsigned int);
    void wb_parameter_set();
    void nb_parameter_set(const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int, const unsigned int);
    boost::system::error_code connectToServer(const std::string&, const unsigned short);
    boost::system::error_code connectToServer();
    bool IsConnected();
    void write(std::unique_ptr<NetCmdData>);
    threadsafe_queue<std::shared_ptr<unsigned char[]>> spsc_queue;

protected:
    void StartWork();

private:
    bool Connected = false;
    void read();
    void write();
    void ReInitSocket();

    unsigned int task_id = 0;
    std::string m_addr;
    short m_port;
    threadsafe_queue<std::unique_ptr<NetCmdData>> write_queue;
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::socket socket;
    std::thread read_thread;
    std::thread write_thread;
};

#endif
