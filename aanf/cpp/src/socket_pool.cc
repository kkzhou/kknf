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

#include "socket_pool.h"
#include "socket.h"
#include "bin_tcp_socket.h"
#include "line_tcp_socket.h"
#include "http_tcp_socket.h"
//#include "bin_udp_socket.h"
//#include "line_udp_socket.h"
//#include "http_udp_socket.h"

namespace AANF {

SocketPool::SocketPool() {
    pthread_mutex_init(&socket_map_lock_, NULL);
}

SocketPool::~SocketPool() {

    // 先销毁所有的套接口
    SweepIdleSocket(-1);    // A trick
    pthread_mutex_destroy(&socket_map_lock_);
}

Socket* SocketPool::FindSocket(SocketKey &key) {

    Locker locker(socket_map_lock_);
    Socket *sk;

    map<SocketKey, list<Socket*>>::iterator it = socket_map_.find(key);

    if (it == socket_map_.end()) {
        sk = NULL;
    } else {
        sk = it->second.pop();
    }
    return sk;
}

int SocketPool::SweepIdleSocket( int max_idle_sec) {

    Locker locker(socket_map_lock_);
    map<Socket, list<Socket*>>::iterator map_it = socket_map_.begin();
    map<Socket, list<Socket*>>::iterator map_endit = socket_map_.end();

    struct timeval nowtime;
    gettimeofday(&nowtime);

    int delted_num = 0;
    while (map_it != map_endit) {

        list<Socket*>::iterator list_it = map_it->second.begin();
        list<Socket*>::iterator list_endit = map_it->second.end();
        while (list_it != list_endit) {
            int64_t delta = nowtime.tv_sec - (*list_it)->last_use_.tv_sec;
            if (delta > max_idle_sec) {
                Socket *tmpsk = *list_it;
                delete tmpsk;
                map_it->erase(it++);
                deleted_num++;
            } else {
                list_it++;
            }
        }// while(list_it)
        if (map_it->second.size() == 0) {
            // 这个list已经是空的了，删除
            socket_map_.erase(++map_it);
        } else {
            map_it++;
        }
    }// while (map_it)

    return deleted_num;
}


int SocketPool::AddSocket(Socket *sk) {

    SocketKey key;

    if (sk->type_ == Socket::T_TCP_CLIENT || sk->type_ == Socket::T_TCP_SERVER) {
        key.ip_ = sk->peer_ipstr_;
        key.port_ = sk->peer_port_;
        key.type_ = sk->type_;
    } else if (sk->type_ == Socket::T_TCP_LISTEN) {
        key.ip_ = sk->my_ipstr_;
        key.port_ = sk->my_port_;
        key.type_ = sk->type_;
    }


    {
        Locker locker(socket_map_lock_);
        map<SocketKey, list<Socket*>>::iterator it = socket_map_.find(key);
        if (it != socket_map_.end()) {
            // 已经存在
            it->second.push_back(sk);
            return 1;
        } else {
            list<Socket*> new_list;
            new_list.push_back(sk);
            pair<map<SocketKey, list<Socket*>>::iterator, bool>::iterator ret_it;
            ret_it = socket_map_.insert(pair<SocketKey, list<Socket*>>(key, new_list));
        }
    }

    return 0;
}


int SocketPool::DestroySocket(Socket *sk) {

    Locker locker(socket_map_lock_);
    // First, delete myself from the socket map
    SocketKey key;
    if (sk->type_ == Socket::T_TCP_CLIENT || sk->type_ == Socket::T_TCP_SERVER) {
        key.ip_ = sk->peer_ipstr_;
        key.port_ = sk->peer_port_;
        key.type_ = sk->type_;
    } else if (sk->type_ == Socket::T_TCP_LISTEN) {
        key.ip_ = sk->my_ipstr_;
        key.port_ = sk->my_port_;
        key.type_ = sk->type_;
    }

    map<SocketKey, list<Socket*>>::iterator map_it = socket_map_.find(key);
    if (map_it == socket_map_.end()) {
        return -2;
    }

    list<Socket*>::iterator list_it = map_it->second.begin();
    list<Socket*>::iterator list_endit = map_it->second.end();

    while (list_it != list_endit) {

        if (sk == *list_it) {
            map_it->erase(++list_it);

        } else {
            list_it++;
        }
    }

    delete sk;
    return 0;
}

Socket* SocketPool::CreateListenSocket(std::string &listen_ip, uint16_t listen_port,
                                       Socket::SocketType type,
                                       Socket::DataFormat data_format) {

    Socket *sk = socket_pool_->FindSocket(listen_ip, listen_port, type);
    if (sk) {
        return -2;
    }

    switch (type) {
    case  T_TCP_LISTEN:
        switch (data_format) {
        case DF_BIN:
            sk = new BinTcpSocket;
            sk->PrepareListenSocket(listen_ip, listen_port, type, data_format);
            AddSocket(sk);
            break;
        case DF_LINE:
            sk = new LineTcpSocket;
            sk->PrepareListenSocket(listen_ip, listen_port, type, data_format);
            AddSocket(sk);
            break;
        case DF_HTTP:
            sk = new HttpTcpSocket;
            sk->PrepareListenSocket(listen_ip, listen_port, type, data_format);
            AddSocket(sk);
            break;
        default:
            break;
        }
        break;
    case T_TCP_CLIENT:
        break;
    case T_TCP_SERVER:
        break;
    case T_UDP_SERVER:
        break;
    case T_UDP_CLIENT:
        break;
    default:
        break;
    }

    return sk;
}

Socket* SocketPool::CreateClientSocket(std::string &server_ip, uint16_t server_port,
                                       Socket::SocketType type,
                                       Socket::DataFormat data_format) {

    Socket *sk = 0;

    switch (type) {
    case  T_TCP_LISTEN:
        break;
    case T_TCP_CLIENT:
        switch (data_format) {
        case DF_BIN:
            sk = new BinTcpSocket;
            sk->PrepareClientSocket(server_ip, server_port, type, data_format);
            AddSocket(sk);
            break;
        case DF_LINE:
            sk = new LineTcpSocket;
            sk->PrepareClientSocket(server_ip, server_port, type, data_format);
            AddSocket(sk);
            break;
        case DF_HTTP:
            sk = new HttpTcpSocket;
            sk->PrepareClientSocket(server_ip, server_port, type, data_format);
            AddSocket(sk);
            break;
        default:
            break;
        }
        break;
    case T_TCP_SERVER:
        break;
    case T_UDP_SERVER:
        break;
    case T_UDP_CLIENT:
        break;
    default:
        break;
    }

    return sk;
}

Socket* SocketPool::CreateServerSocket(int fd, Socket::SocketType type,
                                       Socket::DataFormat data_format) {

    Socket *sk = 0;

    switch (type) {
    case  T_TCP_LISTEN:
        break;
    case T_TCP_CLIENT:
        break;
    case T_TCP_SERVER:
        switch (data_format) {
        case DF_BIN:
            sk = new BinTcpSocket;
            sk->PrepareServerSocket(fd, type, data_format);
            AddSocket(sk);
            break;
        case DF_LINE:
            sk = new LineTcpSocket;
            sk->PrepareServerSocket(fd, type, data_format);
            AddSocket(sk);
            break;
        case DF_HTTP:
            sk = new HttpTcpSocket;
            sk->PrepareServerSocket(fd, type, data_format);
            AddSocket(sk);
            break;
        default:
            break;
        }
        break;
    case T_UDP_SERVER:
        break;
    case T_UDP_CLIENT:
        break;
    default:
        break;
    }

    return sk;
}

} // namespace AANF
