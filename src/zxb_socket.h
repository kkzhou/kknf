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
class Packet;
class SocketOperator;

// All about a socket
class Socket {

friend SocketOperator;
public:
    // According types
    enum SocketType {
        T_TCP_LISTEN = 1,
        T_TCP_CLIENT,
        T_TCP_SERVER,
        T_UDP_CLIENT,
        T_UDP_SERVER
    };
    enum SocketStatus {
        S_NOTREADY = 1,
        S_CLOSED,
        S_ERROR,
        S_ESTABLISHED,
        S_CONNECTING,
        S_LISTEN
    };
    enum Event {
        EV_READ = 1,
        EV_WRITE = 2,
        EV_ERROR = 4
    };
    enum DataFormat {// Determines how to packetize data
        DF_BIN = 1,
        DF_HTTP,
        DF_LINE
    };

public:
    static Socket* FindSocket (std::string &peer_ip, uint16_t peer_port, enum SocketType type);
    static int DestroySocket();
    static Socket* CreateSocket(SocketOperator *op);
    static int SweepIdleSocket(int max_idle_sec);
    int PushDataToSend(MemBlock *mb, int &seq);

private:
    Socket (SocketOperator *op);
    ~Socket();
public:
    int fd_;// The corresponding file descriptor
    enum SocketType type_;
    enum SocketStatus status_;
    enum DataFormat data_format_;
    std::string my_ipstr_;
    uint16_t my_port_;
    std::string peer_ipstr_;
    uint16_t peer_port_;
    uint16_t event_concern_;
    struct timeval last_use_;
    int retry_times_;
    SocketOperator *op_;
    pthread_mutex_t send_mb_list_lock_;

private:
    // static variables
    // No contend condition for socket_map_
    static std::map<std::string, Socket*> socket_map_;// Use string(IP:PORT:TYPE) as key

    // object variables
    // Buffer to receive data before packetized
    Packet *recv_pkt_;
    std::list<Packet*> send_pkt_list_;// The data(organized in Packet) to send
};

};// namespace ZXB
#endif
