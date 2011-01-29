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

#ifndef _ZXB_SOCKET_H_
#define _ZXB_SOCKET_H_

#include <netinet/in.h>

namespace ZXB {

class MemBlock;
class SocketOperator;
class SocketPool;

// All about a socket
// 这个类是描述一个套接口的所有信息，但是不包含操作套接口的函数，例如socket/connect/accept等。
// 也就是说，这个类只是用来存储信息的。
class Socket {

friend SocketOperator;
friend SocketPool;
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
    };
    // 数据基本格式，即如何确定数据包的边界
    enum DataFormat {// Determines how to packetize data
        DF_BIN = 1,
        DF_HTTP,
        DF_LINE
    };

public:
    int PushDataToSend(MemBlock *mb);

public:
    int fd_;// The corresponding file descriptor
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
    SocketOperator *op_;
private:
    pthread_mutex_t send_mb_list_lock_;
    MemBlock *recv_mb_; // Buffer to receive data before packetized
    std::list<MemBlock*> send_mb_list_;// The data(organized in MemBlock) to send
    Socket(SocketOperator *op);
    ~Socket();
};

// Socket factory
// 套接口池类，用于管理套接口，包括查找、创建、删除和清除空闲套接口等操作。
class SocketPool {
public:
    SocketPool();
    ~SocketPool();
    Socket* FindSocket(std::string &peer_ip, uint16_t peer_port, SocketType type);
    int AddSocket(Socket *sk);
    int DestroySocket();
    Socket* CreateSocket(SocketOperator *op);
    int SweepIdleSocket(int max_idle_sec);
private:
    // 会出现竞争状态，因此要加锁
    std::map<std::string, Socket*> socket_map_;// Use string(IP:PORT:TYPE) as key
    pthread_mutex_t socket_map_lock_;
};

};// namespace ZXB
#endif
