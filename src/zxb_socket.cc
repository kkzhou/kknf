 /*
    Copyright (C) <2011>  <ZHOU Xiaobo>

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

#include "zxb_socket.h"
#include "zxb_memblock.h"
#include "zxb_socket_operator.h"
#include "zxb_utils.h"

namespace ZXB {

Socket* Socket::FindSocket(std::string &peer_ip, uint16_t peer_port, enum SocketType type) {

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

int Socket::SweepIdleSocket( int max_idle_sec) {

    // Only one thread does this thing
    map<string, Socket*>::iterator it = socket_map_.begin();
    map<string, Socket*>::iterator endit = socket_map_.end();

    struct timeval nowtime;
    gettimeofday(&nowtime);

    int delted_num = 0;
    while (it != endit) {
        int64_t delta = nowtime.tv_sec - (*it)->second->last_use_.tv_sec;
        if (delta > max_idle_sec) {
            Socket *tmpsk = *it->second;
            Socket::DestroySocket(tmpsk);
            socket_map_.erase(it++);
            deleted_num++;
        } else {
            it++;
        }
    }

    return deleted_num;
}

Socket* Socket::CreateSocket(SocketOperator *op) {
    return new Socket(op);
}

Socket::Socket(SocketOperator *op) {

    op_ = op;
    send_mb_list_lock_ = PTHREAD_MUTEX_INITIALIZER;
    gettimeofday(&last_use_);
    status_ = S_NOTREADY;
}

int Socket::DestroySocket(Socket *sk) {

    // First, delete myself from the socket map

    stringstream tmp_key;
    if (sk->type_ == T_TCP_LISTEN || sk->type_ == T_TCP_CLIENT || sk->type_ == T_UDP_CLIENT) {
        // We use myip:myport as the key in map for these types of socket
        tmp_key << sk->my_ipstr_ << ":" << sk->my_port_ << ":" << sk->type_;
    } else {
        tmp_key << sk->peer_ipstr_ << ":" << sk->peer_port_ << ":" << sk->type_;
    }
    socket_map_.erase(tmp_key.str());

    // Then, release resources of this socket
    close(sk->fd_);
    sk->op_ = NULL;// op_ doesnt' belong to me but I belong to op_

    // Return the recv buf MemBlock
    sk->recv_mb_->Return();

    {// Send buffer list should be locked first

        Locker locker(send_mb_list_lock_);
        // Return the send buf MemBlock
        std::list<MemBlock*>::iterator it = sk->send_mb_list_.begin();
        std::list<MemBlock*>::iterator endit = sk->send_mb_list_.end();

        for (; it != endit; it++) {
            *it->Return();
        }
        delete sk;
    }

    return 0;
}

int Socket::PushDataToSend(MemBlock *mb)
{
    Locker locker(send_mb_list_lock_);
    send_mb_list_.push_back(mb);
    gettimeofday(&last_use_);
    return 0;
}

Socket::~Socket() {
}

};// namespace ZXB
#endif
