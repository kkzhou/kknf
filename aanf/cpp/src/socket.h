 /*
    Copyright (C) <2011>  <ZHOU Xiaobo(zhxb.ustc@gmail.com)>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <netinet/in.h>
#include <string>
#include <list>
#include <map>

namespace AANF {

class MemBlock;

// 这个类是描述一个套接口的所有信息，但是不包含操作套接口的函数，例如socket/connect/accept等。
// 也就是说，这个类只是用来存储信息的。
class Socket {

public:
    // 套接口类型，按功能区分（我们并不试图创造一个万能的网络库，因此只针对常用场景来实现），
    enum SocketType {
        T_TCP_LISTEN = 1,   // 用于侦听的TCP套接口，即用于accept，得到新套接口
        T_TCP_CLIENT,       // 主动发起连接的TCP套接口
        T_TCP_SERVER,       // 由侦听套接口accept时产生的套接口
        T_UDP_CLIENT,       // 主动发起数据发送的UDP套接口
        T_UDP_SERVER        // 用于侦听的UDP套接口，不同于TCP，这里的侦听其实就是接收数据。
    };

    // 套接口的状态
    enum SocketStatus {
        S_NOTREADY = 1,     // 刚刚创建时，还没有开始真正的套接口操作，只是被实例化了一个对象而已
        S_CLOSED,           // 套接口处于关闭状态
        S_ERROR,            // 出现错误
        S_ESTABLISHED,      // 处于正常的数据通信状态（UDP也可以用这个状态）
        S_CONNECTING,       // TCP处于连接状态，即connect的INPROCESS状态
        S_LISTEN            // 侦听套接口开始侦听
    };
    // 套接口的事件，只是epoll状态的子集，需要进行转换
    // 可以按位或
    enum Event {
        EV_READ = 1,
        EV_WRITE = 2,
        EV_ERROR = 4
        EV_PERSIST = 8
    };
    // 数据基本格式，即如何确定数据包的边界
    enum DataFormat {// Determines how to packetize data
        DF_BIN = 1,
        DF_HTTP = 2,
        DF_LINE = 3
    };

public:
    virtual int PrepareListenSocket(std::string &listen_ip, uint16_t listen_port,
                           Socket::SocketType type,
                           Socket::DataFormat data_format) = 0;
    virtual int PrepareClientSocket(std::string &server_ip, uint16_t server_port,
                           Socket::SocketType type,
                           Socket::DataFormat data_format) = 0;
    virtual int PrepareServerSocket(int fd, Socket::SocketType type,
                           Socket::DataFormat data_format) = 0;
    virtual int ReadHandler() = 0;
    virtual int WriteHandler() = 0;
    virtual int Reconnect() = 0;

    int PushDataToSend(MemBlock *mb);
    int GetSocketError(int &error);
    uint16_t event_concern();
    void set_event_concern(uint16_t events);
    uint16_t add_event_to_concern(Event ev);
    uint16_t del_event_to_concern(Event ev);

public:
    int fd_;
    SocketType type_;
    SocketStatus status_;
    DataFormat data_format_;

    std::string my_ipstr_;
    uint16_t my_port_;
    std::string peer_ipstr_;
    uint16_t peer_port_;

    uint16_t event_concern_;

    struct timeval last_use_;
    int retry_times_;

private:
    MemBlock *recv_mb_; // Buffer to receive data before packetized
    std::list<MemBlock*> send_mb_list_;// The data(organized in MemBlock) to send
    int font_mb_cur_pos_;   // 队列最前面的MemBlcok已发送的字节数
    Socket();
    ~Socket();
};

};// namespace AANF
#endif
