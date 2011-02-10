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
#include "zxb_socket.h"

namespace ZXB{

SocketOperator::SocketOperator() :socket_(0) {
}

SocketOperator::~SocketOperator() {

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

int SocketOperator::ErrorHandler(SocketCmd cmd) {

    if (cmd = C_RECONNECT) {
        // prepare socket
        if (socket_->fd_ = socket(PF_INET, SOCK_STREAM, 0) < 0) {
            perror("socket() error: ");
            return -2;
        }

        // Set nonblocking
        int val = fcntl(socket_->fd_ ->fd_, F_GETFL, 0);
        if (val == -1) {
            perror("Get socket flags error: ");
            return -2;
        }

        if (fcntl(socket_->fd_, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
            perror("Set socket flags error: ");
            return -2;
        }
        // prepare address
        struct sockaddr_in to_addr;
        to_addr.sin_family = AF_INET;
        to_addr.sin_port = htons(socket_->peer_port_);
        if (inet_aton(socket_->peer_ipstr_.c_str(), &to_addr.sin_addr) == 0) {
            return -2
        }
        // connect
        if (connect(socket_->fd_, (struct sockaddr*)&to_addr, sizeof(struct sockaddr)) == -1) {
            if (errno != EINPROGRESS) {
                perror("Connect error: ");
                return -2
            }
        }

        socket_->status_ = Socket::S_CONNECTING;
    } else if (cmd == C_CLOSE) {
        return 0;
    } else if (cmd == C_SHUTDOWN) {
    } else {
        return -1;
    }
    return 1;
}

}; // namespace ZXB



