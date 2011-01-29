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

#include <sys/types.h>
#include <sys/socket.h>

#include "zxb_socket_operator.h"

SocketOperator::SocketOperator() :socket_(0) {
}

SocketOperator::~SocketOperator() {

    Socket::Destory(socket_);
}

int SocketOperator::set_socket(Socket *sk) {

    if (!sk) {
        return -1;
    }
    if (socket_) {
        return -2;
    }
    socket_ = sk;
    return 0;
}

Socket* SocketOperator::socket() {
    return socket_;
}

// return:
// -1: parameters invalid
// -2: socket exists
int SocketOperator::PrepareListenSocket(std::string &my_ipstr,
                                        uint16_t my_port,
                                        enum SocketType type) {

    // Check parameters
    if (my_ipstr.empty() || type != Socket::T_TCP_LISTEN) {
        return -1;
    }

    Socket *sk = SocketFindSocket(my_ipstr, my_port, type);
    if (sk) {
        return -2;
    }
    sk = Socket::Create(this);
    sk->my_ipstr_ = my_ipstr;
    sk->my_port_ = my_port;
    sk->type_ = type;
    sk->event_concern_ = Socket::EV_READ | Socket::EV_ERROR;

    // prepare socket
    if (sk->fd_ = socket(PF_INET, SOCK_STREAM, 0) < 0) {
        perror("socket() error: ");
        Socket::Destroy(sk);
        return -3;
    }
    // Set nonblocking
    int val = fcntl(sk->fd_, F_GETFL, 0);
    if (val == -1) {
        perror("Get socket flags error: ");
        Socket::Destroy(sk);
        return -3;
    }
    if (fcntl(sk->fd_, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
        perror("Set socket flags error: ");
        Socket::Destroy(sk);
        return -3;
    }
    // Bind address
    struct sockaddr_in listen_addr;
    //listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(sk->my_port);
    if (inet_aton(sk->my_ipstr_.c_str(), &listen_addr.sin_addr) ==0) {
        Socket::Destroy(sk);
        return -3;
    }
    socket_len addr_len = sizeof(struct sockaddr_in);
    if (bind(sk->fd_, (struct sockaddr*)listen_addr, addr_Len) == -1) {
        Socket::Destroy(sk);
        return -3;
    }
    // Set address reusable
    int optval = 0;
    size_t optlen = sizeof(optval);
    if (setsockopt(sk->fd_, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0) {
        perror("Set address reusable error:");
        Socket::Destroy(sk);
        return -3;
    }
    // Listen
    if (listen(sk->fd_, 1024) == -1) {
        perror("Listen socket error: ");
        return -3;
    }
    // Asynchronously accept
    if (accept(sk->fd_, NULL, 0) == -1) {
        if (errno != EAGAIN || errno != EWOULDBLOCK) {
            perror("Async accept error: ");
            Socket::Destroy(sk);
            return -3;
        }
    }
    socket_ = sk;
    return 0;
}

int SocketOperator::GetSocketError(int fd, int &error)
{
    error = 0;
    socklen_t optlen = sizeof(error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &optlen) < 0)
        return -2;

    return 0;
}

int SocketOperator::AsyncSend(std::string &to_ipstr, uint16_t to_port,
                              std::string &my_ipstr, uint16_t my_port, int &seq,
                              MemBlock *data, enum SocketType type,
                              SocketOperator *&sk_used) {

    // Check parameters
    //if (my_ipstr.empty() || to_ipstr.empty() || to_port == 0
    //    || (type != Socket::T_TCP_CLIENT && type != Socket::T_UDP_CLIENT) {

    if (to_ipstr.empty() || to_port == 0 ||
        (type != Socket::T_TCP_CLIENT && type != Socket::T_UDP_CLIENT)) {

        return -1;
    }

    Socket *sk = SocketFindSocket(my_ipstr, my_port, type);
    if (sk) {
        // The socket alreay exists, just send to it
        sk->PushDataToSend(data);// It should be thread-safe
        return 0;
    }
    // Create a new one which should act as CLINET
    sk = Socket::CreateSocket(this);
    sk->my_ipstr_ = my_ipstr;
    sk->my_port_ = my_port;
    sk->type_ = type;
    sk->event_concern_ = Socket::EV_READ | Socket::EV_WRITE | Socket::EV_ERROR;

    // prepare socket
    if (sk->fd_ = socket(PF_INET, SOCK_STREAM, 0) < 0) {
        perror("socket() error: ");
        Socket::DestroySocket(sk);
        return -3;
    }
    // Set nonblocking
    int val = fcntl(sk->fd_, F_GETFL, 0);
    if (val == -1) {
        perror("Get socket flags error: ");
        Socket::DestroySocket(sk);
        return -3;
    }
    if (fcntl(sk->fd_, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
        perror("Set socket flags error: ");
        Socket::DestroySocket(sk);
        return -3;
    }

    // prepare address
    struct sockaddr_in to_addr;
    to_addr.sin_family = AF_INET;
    to_addr.sin_port = htons(to_port);
    if (inet_aton(to_ipstr.c_str(), &to_addr.sin_addr) == 0) {
        Socket::DestroySocket(sk);
        return -3;
    }
    // connect
    if (connect(sk->fd_, (struct sockaddr*)&to_addr, sizeof(struct sockaddr)) == -1) {
        if (errno != EINPROGRESS) {
            perror("Connect error: ");
            Socket::DestroySocket(sk);
            return -3;
        }
    }
    // Push data to send
    sk->PushDataToSend(data, seq);
    sk->status_ = Socket::S_CONNECTING;
    sk_used = sk;
    return 0;
}

int SocketOperator::AsyncRecv(std::string &my_ipstr, uint16_t my_port,
                              enum SocketType type, SocketOperator *&sk_used) {
    return 0;
}

int SocketOperator::ReadHandler(Packet *&in_pack) {
    int ret = 0;
    if (socket_->data_format_ == Socket::DF_BIN) {
        // Bin packet is: length(4bytes)+sequence(4bytes)+data
        ret = BinReadHandler(in_pack);
    } else if (socket_->data_format_ == Socket::DF_HTTP) {
        // Http packet
        ret = HttpReadHandler(in_pack);
    } else if (socket_->data_format_ == Socket::DF_LINE) {
        // Line packet
        ret = LineReadHandler(in_pack);
    } else {
        ret = OtherReadHandler(in_pack);
    }
    return ret;
}

int SocketOperator::BinReadHandler(Packet *&in_pack) {

    if (socket_->recv_pkt_) {

    }
}

