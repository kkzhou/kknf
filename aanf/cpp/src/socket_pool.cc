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
//#include "bin_tcp_socket.h"
//#include "line_tcp_socket.h"
//#include "http_tcp_socket.h"

namespace AANF {

SocketPool::SocketPool() {
    pthread_mutex_init(&socket_map_lock_, NULL);
}

SocketPool::~SocketPool() {
    SweepIdleSocket(-1);    // A trick
    pthread_mutex_destroy(&socket_map_lock_);
}

Socket* SocketPool::FindSocket(std::string &peer_ip, uint16_t peer_port, SocketType type) {

    Locker locker(socket_map_lock_);
    stringstream tmp_key;
    if (type_ == T_TCP_LISTEN || type_ == T_TCP_CLIENT || type_ == T_UDP_CLIENT) {
        // We use myip:myport as the key in map for these types of socket
        return NULL;
    } else {
        tmp_key << peer_ip << ":" << peer_port << ":" << type;
    }
    map<string, Socket*>::iterator it = socket_map_.find(tmp_key.str());
    if (it == socket_map_.end()) {
        return NULL;
    }
    return it->second;
}

int SocketPool::SweepIdleSocket( int max_idle_sec) {

    Locker locker(socket_map_lock_);
    map<string, Socket*>::iterator it = socket_map_.begin();
    map<string, Socket*>::iterator endit = socket_map_.end();

    struct timeval nowtime;
    gettimeofday(&nowtime);

    int delted_num = 0;
    while (it != endit) {
        int64_t delta = nowtime.tv_sec - (*it)->second->last_use_.tv_sec;
        if (delta > max_idle_sec) {
            Socket *tmpsk = *it->second;
            delete tmpsk;
            socket_map_.erase(it++);
            deleted_num++;
        } else {
            it++;
        }
    }

    return deleted_num;
}


int SocketPool::AddSocket(Socket *sk) {

    stringstream tmp_key;
    // check sk
    if (sk->type_ == Socket::T_TCP_SERVER) {
        // check peer_ip/peer_port
        if (sk->peer_ipstr_.empty() || sk->peer_port_ == 0) {
            return -1;
        }
        tmp_key << sk->peer_ipstr_ << ":" << sk->peer_port_ << ":" << sk->type_;
    } else if (sk->type_ == Socket::T_TCP_LISTEN || sk->type_ == Socket::T_TCP_CLIENT) {
        // check my_ip/my_port
        if (sk->my_ipstr_.empty() || sk->my_port_ == 0) {
            return -1;
        }
        tmp_key << sk->my_ipstr_ << ":" << sk->my_port_ << ":" << sk->type_;
    } else {
        return -1;
    }

    {
        Locker locker(socket_map_lock_);
        if (socket_map_.find(tmp_key.str()) != socket_map_.end()) {
            // already exist
            return -2;
        }

        pair<map<string, Socket*>::iterator, bool>::iterator it;
        it = socket_map_.insert(pair<string, Socket*>(tmp_key.str(), sk)));
    }
    return 0;
}


int SocketPool::DestroySocket(Socket *sk) {

    Locker locker(socket_map_lock_);
    // First, delete myself from the socket map
    stringstream tmp_key;
    if (sk->type_ == T_TCP_LISTEN || sk->type_ == T_TCP_CLIENT || sk->type_ == T_UDP_CLIENT) {
        // We use myip:myport as the key in map for these types of socket
        tmp_key << sk->my_ipstr_ << ":" << sk->my_port_ << ":" << sk->type_;
    } else {
        tmp_key << sk->peer_ipstr_ << ":" << sk->peer_port_ << ":" << sk->type_;
    }
    socket_map_.erase(tmp_key.str());
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
