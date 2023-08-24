#include "TcpSocket.h"

#include <iostream>
#include "StructNetData.h"
#include "global.h"

extern PARAMETER_SET g_parameter_set;

TcpSocket::TcpSocket(): spsc_queue(isRunning), write_queue(isRunning), socket(ioService)
{
    ReInitSocket();
}

TcpSocket::~TcpSocket()
{
    isRunning = false;
    write_queue.clean();
    if (read_thread.joinable())
        read_thread.join();
    if (write_thread.joinable())
        write_thread.join();
}

void TcpSocket::ReInitSocket()
{
    socket = boost::asio::ip::tcp::socket(ioService);
    boost::system::error_code err_code;
    socket.set_option(boost::asio::socket_base::send_buffer_size(4 * 1024 * 1024), err_code);
}

bool TcpSocket::IsConnected()
{
    return isRunning;
}

void TcpSocket::StartWork()
{
    read_thread = std::thread([this]
    {
        while (isRunning)
        {
            read();
        }
        std::cout << "Read Thread Exit" << std::endl;
        write_queue.clean();
    });

    write_thread = std::thread([this]
    {
        while (isRunning)
        {
            write();
        }
        std::cout << "Write Thread Exit" << std::endl;
    });
}

void TcpSocket::read()
{
    try
    {
        DataHead head;
        boost::system::error_code ec;
        size_t left = sizeof(DataHead), offset = 0;
        while (left > 0)
        {
            auto bytes_transferred = boost::asio::read(socket, boost::asio::buffer((char*)&head + offset, left), ec);
            if (ec.failed())
            {
                std::cout << "Read Failed: " << ec.what() << std::endl;
                isRunning = false;
                return;
            }
            left -= bytes_transferred;
            offset += bytes_transferred;
        }
        left = head.PackLen - sizeof(DataHead), offset = sizeof(DataHead);
        // auto data = std::make_shared<char[]>(block_size);
        auto tmp = new unsigned char[head.PackLen];
        *(DataHead*)tmp = head;
        auto data = std::shared_ptr<unsigned char[]>(tmp);
        while (left > 0)
        {
            auto bytes_transferred = boost::asio::read(socket, boost::asio::buffer(data.get() + offset, left), ec);
            if (ec.failed())
            {
                std::cout << "Read Failed: " << ec.what() << std::endl;
                isRunning = false;
                return;
            }
            left -= bytes_transferred;
            offset += bytes_transferred;
        }
        spsc_queue.push(data);
    }
    catch (const std::exception& e)
    {
        std::cout << "Read Exception:" << e.what() << std::endl;
        isRunning = false;
    }
}

void TcpSocket::write(std::unique_ptr<NetCmdData> data)
{
    write_queue.push(std::move(data));
}

void TcpSocket::write()
{
    try
    {
        auto data = write_queue.wait_and_pop();
        if (!isRunning)
            return;
        boost::system::error_code ec;
        size_t left = data->len, offset = 0;
        while (left > 0)
        {
            auto bytes_transferred = boost::asio::write(socket, boost::asio::buffer(data->data + offset, left), ec);
            if (ec.failed())
            {
                std::cout << "Write Failed: " << ec.what() << std::endl;
                isRunning = false;
                return;
            }
            left -= bytes_transferred;
            offset += bytes_transferred;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Write Exception: " << e.what() << std::endl;
        isRunning = false;
    }
}

void TcpSocket::self_check(const unsigned int mode)
{
    char tmp[100];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0101;SCheck:%d\r\n", task_id, mode);
    write(std::make_unique<NetCmdData>(std::string(tmp)));
}

void TcpSocket::work_ctrl(const unsigned int mode)
{
    char tmp[100];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0102;WorkCtrl:%d\r\n", task_id, mode);
    write(std::make_unique<NetCmdData>(std::string(tmp)));
}

void TcpSocket::parameter_set()
{
    char tmp[200];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0103;CenterFreq:%lld;SimBW:%d;FreqRes:%d;SmNum:%d;Feedback:%d;GainMode:%d;Rf_MGC:%d;Digit_MGC:%d\r\n",
                 task_id, g_parameter_set.CenterFreq, g_parameter_set.SimBW, g_parameter_set.FreqRes, g_parameter_set.SmNum,
                 g_parameter_set.Feedback, g_parameter_set.GainMode, g_parameter_set.Rf_MGC, g_parameter_set.Digit_MGC);
    write(std::make_unique<NetCmdData>(std::string(tmp)));
}

void TcpSocket::nb_receiver(const unsigned int receiver, const unsigned int freq_Hz)
{
    char tmp[200];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0403;RcvNum:%d;Freq:%d;Mod:1;BW:200;GMode:1;MGC:0;SQU:0;ATT:0;Vol:0;AFC:0;RcvMode:0\r\n",
                 task_id, receiver, freq_Hz);
    write(std::make_unique<NetCmdData>(std::string(tmp)));
}

void TcpSocket::nb_channel(const unsigned int receiver, const unsigned int channel, const unsigned int freq_Hz, const unsigned int bandwidth)
{
    char tmp[200];
    std::sprintf(tmp, "Task:%d;Sta:2;Type:0411;RcvNum:%d;BankNum:%d;GMode:0;MGC:30;KpTime:2;Sn:1;Freq:%d;DDCBW:%d;DemodType:1;DemodRate:2000;\r\n",
                 task_id, receiver, channel, freq_Hz, bandwidth);
    write(std::make_unique<NetCmdData>(std::string(tmp)));
}

boost::system::error_code TcpSocket::connectToServer(const std::string& addr, const unsigned short port)
{
    m_addr = std::move(addr);
    m_port = port;
    return connectToServer();
}

boost::system::error_code TcpSocket::connectToServer()
{
    boost::system::error_code ec;
    if (isRunning)
    {
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        if (ec.failed())
        {
            std::cout << "socket shutdown error: " << ec.what() << std::endl;
            return ec;
        }
        socket.close(ec);
        if (ec.failed())
        {
            std::cout << "socket close error: " << ec.what() << std::endl;
            return ec;
        }
    }
    if (read_thread.joinable())
        read_thread.join();
    if (write_thread.joinable())
        write_thread.join();
    ReInitSocket();
    socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(m_addr), m_port), ec);
    if (ec.failed())
    {
        std::cout << ec.what() << std::endl;
        return ec;
    }
    isRunning = true;
    StartWork();
    return ec;
}
