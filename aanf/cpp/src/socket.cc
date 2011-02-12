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

#include "zxb_socket.h"
#include "zxb_memblock.h"
#include "zxb_socket_operator.h"
#include "zxb_utils.h"

namespace ZXB {

// 构造函数
Socket::Socket(SocketOperator *op) {

    op_ = op;
    pthread_mutex_init(&send_mb_list_lock_, NULL);
    gettimeofday(&last_use_);
    status_ = S_NOTREADY;
    font_mb_cur_pos_ = 0;
}
// 析构函数
// 需要释放资源，包括接受缓存、发送缓存列表
Socket::~Socket() {

    close(fd_);
    delete op_;

    // Return the recv buf MemBlock
    recv_mb_->Return();
    recv_mb_ = 0;
    std::list<MemBlock*>::iterator it = send_mb_list_.begin();
    std::list<MemBlock*>::iterator endit = send_mb_list_.end();

    for (; it != endit; it++) {
        *it->Return();
    }
}

int Socket::PushDataToSend(MemBlock *mb)
{
    send_mb_list_.push_back(mb);
    gettimeofday(&last_use_);
    return 0;
}

int Socket::GetSocketError(int fd, int &error)
{
    error = 0;
    socklen_t optlen = sizeof(error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &optlen) < 0)
        return -2;

    return 0;
}


SocketPool *SocketPool::socket_pool_ = 0;

SocketPool::SocketPool() {
    pthread_mutex_init(&socket_map_lock_, NULL);
}

SocketPool::~SocketPool() {
    SweepIdleSocket(-1);    // A trick
    pthread_mutex_destroy(&socket_map_lock_);
}

SocketPool* SocketPool::GetSocketPool() {
    if (socket_pool_) {
        return socket_pool_;
    }
    socket_pool_ = new SocketPool;
    return socket_pool_;
}

void SocketPool::DestroySocketPool() {
    delete socket_pool_;
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

Socket* SocketPool::CreateSocket(SocketOperator *op) {
    Socket *newsk = new Socket(op);
    return newsk;
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

};// namespace ZXB
#endif
