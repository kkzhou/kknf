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

#include "tcp_socket.h"

namespace AANF {

int TcpSocket::PrepareListenSocket(std::string &listen_ip, uint16_t listen_port,
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


int TcpSocket::PrepareClientSocket(std::string &server_ip, uint16_t server_port,
                                      Socket::SocketType type,
                                      Socket::DataFormat data_format) {

    // Check parameters
    if (server_ip.empty() || type != Socket::T_TCP_CLIENT) {
        return -1;
    }

    peer_ipstr_ = server_ip;
    peer_port_ = server_port;
    type_ = type;
    data_format_ = data_format;
    event_concern_ = Socket::EV_READ | Socket::EV_ERROR;

    // prepare socket
    if (fd_ = socket(PF_INET, SOCK_STREAM, 0) < 0) {
        perror("socket() error: ");
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

    struct sockaddr_in server_addr;
    //server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_);
    if (inet_aton(peer_ipstr_.c_str(), &server_addr.sin_addr) ==0) {
        return -3;
    }
    socket_len addr_len = sizeof(struct sockaddr_in);

    // Asynchronously connect
    if (connect(fd_, (struct sockaddr*)&server_addr, addr_len) == -1) {
        if (errno != EAGAIN || errno != EWOULDBLOCK) {
            perror("Async connect error: ");
            return -3;
        }
    }

    // Get my_ip of this socket
    struct sockaddr_in myaddr;
    sockaddr_len myaddr_len;
    if (getsockname(fd_, (struct sockaddr*)&myaddr, &myaddr_len) == -1) {
        perror("getsockname error: ");
        return -3;
    }

    my_ipstr_ = inet_ntoa(&myaddr.sin_addr);
    my_port_ = ntohs(myaddr.sin_port);
    if (my_ipstr_.empty()) {
        return -3;
    }
    return 0;
}
int TcpSocket::PrepareServerSocket(int fd, Socket::SocketType type,
                           Socket::DataFormat data_format) {

    fd_ = fd;
    type_ = type;
    data_format_ = data_format;
    event_concern_ = Socket::EV_READ | Socket::EV_ERROR;

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
    // Get my_ip of this socket
    struct sockaddr_in myaddr;
    sockaddr_len myaddr_len;
    if (getsockname(fd_, (struct sockaddr*)&myaddr, &myaddr_len) == -1) {
        perror("getsockname error: ");
        return -3;
    }

    my_ipstr_ = inet_ntoa(&myaddr.sin_addr);
    my_port_ = ntohs(myaddr.sin_port);
    if (my_ipstr_.empty()) {
        return -3;
    }

    // Get peer_ip of this socket
    struct sockaddr_in peer_addr;
    sockaddr_len peer_addr_len;
    if (getpeername(fd_, (struct sockaddr*)&peer_addr, &peer_addr_len) == -1) {
        perror("getpeername error: ");
        return -3;
    }

    peer_ipstr_ = inet_ntoa(&peer_addr.sin_addr);
    peer_port_ = ntohs(peer_addr.sin_port);
    if (peer_ipstr_.empty()) {
        return -3;
    }

    return 0;

}
}; // namespace AANF
