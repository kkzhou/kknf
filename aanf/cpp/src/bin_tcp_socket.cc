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

#include "bin_tcp_socket.h"

namespace AANF {

int BinTcpSocket::PrepareListenSocket(std::string &listen_ip, uint16_t listen_port,
                                      Socket::SocketType type,
                                      Socket::DataFormat data_format) {

    // Check parameters
    if (listen_ip.empty() || type != Socket::T_TCP_LISTEN) {
        return -1;
    }

    my_ipstr_ = listen_ip;
    my_port_ = listen_port;
    type_ = type;
    data_format_ = data_format;
    event_concern_ = Socket::EV_READ | Socket::EV_ERROR;

    // prepare socket
    if (fd_ = socket(PF_INET, SOCK_STREAM, 0) < 0) {
        perror("socket() error: ");
        socket_pool_->DestroySocket(sk);
        return -3;
    }
    // Set nonblocking
    int val = fcntl(fd_, F_GETFL, 0);
    if (val == -1) {
        perror("Get socket flags error: ");
        return -3;
    }
    if (fcntl(fd_, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
        perror("Set socket flags error: ");
        return -3;
    }
    // Bind address
    struct sockaddr_in listen_addr;
    //listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(my_port_);
    if (inet_aton(my_ipstr_.c_str(), &listen_addr.sin_addr) ==0) {
        return -3;
    }
    socket_len addr_len = sizeof(struct sockaddr_in);
    if (bind(fd_, (struct sockaddr*)listen_addr, addr_len) == -1) {
        return -3;
    }
    // Set address reusable
    int optval = 0;
    size_t optlen = sizeof(optval);
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0) {
        perror("Set address reusable error:");
        return -3;
    }
    // Listen
    if (listen(fd_, 1024) == -1) {
        perror("Listen socket error: ");
        return -3;
    }
    // Asynchronously accept
    if (accept(fd_, NULL, 0) == -1) {
        if (errno != EAGAIN || errno != EWOULDBLOCK) {
            perror("Async accept error: ");
            return -3;
        }
    }

    return 0;

}
}; // namespace AANF
