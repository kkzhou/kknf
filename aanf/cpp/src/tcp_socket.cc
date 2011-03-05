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
#include "utils.h"

namespace AANF {

int TcpSocket::PrepareListenSocket(std::string &listen_ip, uint16_t listen_port,
                                      Socket::SocketType type,
                                      Socket::DataFormat data_format) {

    ENTERING;
    // Check parameters
    if (listen_ip.empty() || type != Socket::T_TCP_LISTEN) {
        SLOG(LogLevel::L_LOGICERR, "parameter invalid\n");
        LEAVING;
        return -1;
    }

    my_ipstr_ = listen_ip;
    my_port_ = listen_port;
    type_ = type;
    data_format_ = data_format;

    // prepare socket
    if (fd_ = socket(PF_INET, SOCK_STREAM, 0) < 0) {
        SLOG(LogLevel::L_SYSERR, "socket() error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }
    // Set nonblocking
    int val = fcntl(fd_, F_GETFL, 0);
    if (val == -1) {
        SLOG(LogLevel::L_SYSERR, "fcntl(F_GETFL) error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }
    if (fcntl(fd_, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
        SLOG(LogLevel::L_SYSERR, "fcntl(F_SETFL error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }
    // Bind address
    struct sockaddr_in listen_addr;
    //listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(my_port_);
    if (inet_aton(my_ipstr_.c_str(), &listen_addr.sin_addr) ==0) {
        SLOG(LogLevel::L_SYSERR, "inet_aton() error\n");
        LEAVING;
        return -3;
    }
    socket_len addr_len = sizeof(struct sockaddr_in);
    if (bind(fd_, (struct sockaddr*)listen_addr, addr_len) == -1) {
        SLOG(LogLevel::L_SYSERR, "bind() error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }
    // Set address reusable
    int optval = 0;
    size_t optlen = sizeof(optval);
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0) {
        SLOG(LogLevel::L_SYSERR, "setsockopt() error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }
    // Listen
    if (listen(fd_, 1024) == -1) {
        SLOG(LogLevel::L_SYSERR, "listen() error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }
    LEAVING;
    return 0;

}


int TcpSocket::PrepareClientSocket(std::string &server_ip, uint16_t server_port,
                                      Socket::SocketType type,
                                      Socket::DataFormat data_format) {

    ENTERING;
    // Check parameters
    if (server_ip.empty() || type != Socket::T_TCP_CLIENT) {
        SLOG(LogLevel::L_LOGICERR, "parameter invalid\n");
        LEAVING;
        return -1;
    }

    peer_ipstr_ = server_ip;
    peer_port_ = server_port;
    type_ = type;
    data_format_ = data_format;

    // prepare socket
    if (fd_ = socket(PF_INET, SOCK_STREAM, 0) < 0) {
        SLOG(LogLevel::L_SYSERR, "socket() error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }
    // Set nonblocking
    int val = fcntl(fd_, F_GETFL, 0);
    if (val == -1) {
        SLOG(LogLevel::L_SYSERR, "fcntl(F_GETFL) error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }
    if (fcntl(fd_, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
        SLOG(LogLevel::L_SYSERR, "fcntl(F_SETFL) error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }

    struct sockaddr_in server_addr;
    //server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(peer_port_);
    if (inet_aton(peer_ipstr_.c_str(), &server_addr.sin_addr) ==0) {
        SLOG(LogLevel::L_SYSERR, "inet_aton error\n");
        LEAVING;
        return -3;
    }
    socket_len addr_len = sizeof(struct sockaddr_in);

    // Asynchronously connect
    if (connect(fd_, (struct sockaddr*)&server_addr, addr_len) == -1) {
        if (errno != EAGAIN || errno != EWOULDBLOCK) {
            SLOG(LogLevel::L_SYSERR, "inet_aton error\n");
            LEAVING;
            return -3;
        }
    }

    // Get my_ip of this socket
    struct sockaddr_in myaddr;
    sockaddr_len myaddr_len;
    if (getsockname(fd_, (struct sockaddr*)&myaddr, &myaddr_len) == -1) {
        SLOG(LogLevel::L_SYSERR, "getsockname() error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }

    my_ipstr_ = inet_ntoa(&myaddr.sin_addr);
    my_port_ = ntohs(myaddr.sin_port);
    if (my_ipstr_.empty()) {
        SLOG(LogLevel::L_SYSERR, "inet_ntoa error\n");
        LEAVING;
        return -3;
    }
    LEAVING;
    return 0;
}

int TcpSocket::PrepareServerSocket(int fd, Socket::SocketType type,
                           Socket::DataFormat data_format) {

    ENTERING;
    fd_ = fd;
    type_ = type;
    data_format_ = data_format;
    // Set nonblocking
    int val = fcntl(fd_, F_GETFL, 0);
    if (val == -1) {
        SLOG(LogLevel::L_SYSERR, "fcntl(F_GETFL) error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }
    if (fcntl(fd_, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
        SLOG(LogLevel::L_SYSERR, "fcntl(F_SETFL) error: %s\n", strerror(errno));
        LEAVING;
        return -3;
    }
    // Get my_ip of this socket
    struct sockaddr_in myaddr;
    sockaddr_len myaddr_len;
    if (getsockname(fd_, (struct sockaddr*)&myaddr, &myaddr_len) == -1) {
        SLOG(LogLevel::L_SYSERR, "getsockname() error: %s\n", strerror(errno));
        LEAVING;
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
        SLOG(LogLevel::L_SYSERR, "inet_ntoa error\n");
        LEAVING;
        return -3;
    }

    peer_ipstr_ = inet_ntoa(&peer_addr.sin_addr);
    peer_port_ = ntohs(peer_addr.sin_port);
    if (peer_ipstr_.empty()) {
        SLOG(LogLevel::L_SYSERR, "inet_ntoa error\n");
        LEAVING;
        return -3;
    }
    LEAVING;
    return 0;

}

// 实际实现对套接口的写操作的函数。因为有一个链表维护着要发送的内存块
// 我们采用writev函数。
// 返回值：
// 0: 未把队列中的所有数据写完
// 1: 已把队列中的数据写完，应该把该套接口从epoll中去除
// <0: 出现错误，应该关闭或者重连
int TcpSocket::WriteHandler() {

    ENTERING;
    list<MemBlock*>::iterator it = send_mb_list_.begin();
    list<MemBlock*>::iterator endit = send_mb_list_.end();

    struct iovec vec[MAX_IOVEC_NUM];
    int cnt = 0;
    for (; it != endit; it++) {
        vec[cnt].iov_base = *it->start_;
        vec[cnt].iov_len = *it->used_;
        cnt++;
        if (cnt == MAX_IOVEC_NUM - 1) {
            break;
        }
    }

    int write_num = writev(fd_, vec, cnt);
    if (write_num <= 0) {

        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0) {
            // 表明该套接口不可写，应该来说不会达到这里
            SLOG(LogLevel::L_SYSERR, "writev error: %s\n", strerror(errno));
            LEAVING;
            return 0;
        }
        // 否则，说明套接口出现错误
        SLOG(LogLevel::L_SYSERR, "writev error: %s\n", strerror(errno));
        LEAVING;
        return -2;
    }

    int tmp_num = write_num;
    for (int i = 0; i <= cnt; i++) {
        if (tmp_num >= vec[i].iov_len) {
            // 这个块已经发送出去了
            MemBlock *tmpmb = send_mb_list_.front();
            send_mb_list_.pop_front();
            MemPool::GetMemPool()->ReturnMemBlock(tmpmb);
            tmp_num -= vec[i].len;
        } else {
            // 这个块没有发送完
            font_mb_cur_pos_ = tmp_num;
            break;
        }
    }
    if (send_mb_list_.size() == 0 ) {
        // 应该从可写的监控中去掉
        SLOG(LogLevel::L_DEBUG, "send buffer is empty\n");
        LEAVING;
        return 1;
    }
    LEAVING;
    return 0;
}


int TcpSocket::Reconnect() {

    ENTERING;
    if (type_ != Socket::T_TCP_CLIENT) {
        SLOG(LogLevel::L_LOGICERR, "not a tcp client socket\n");
        return -1;
    }
    struct sockaddr_in server_addr;
    //server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_);
    if (inet_aton(peer_ipstr_.c_str(), &server_addr.sin_addr) == 0) {
        SLOG(LogLevel::L_SYSERR, "inet_aton() error\n");
        LEAVING;
        return -3;
    }
    socket_len addr_len = sizeof(struct sockaddr_in);

    // Asynchronously connect
    if (connect(fd_, (struct sockaddr*)&server_addr, addr_len) == -1) {
        SLOG(LogLevel::L_DEBUG, "connect() error\n");
        if (errno != EAGAIN || errno != EWOULDBLOCK) {
            SLOG(LogLevel::L_SYSERR, "connect() error\n");
            LEAVING;
            return -3;
        }
    }
    LEAVING;
    return 0;
}
}; // namespace AANF
