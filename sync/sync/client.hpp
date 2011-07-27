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

#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

#include "util.hpp"
#include "socket.hpp"

namespace NF {

class Client {

public:
    // 阻塞的创建一个连接，然后设置成非阻塞
    Socket* MakeConnect(std::string &ip, uint16_t port) {

        ENTERING;
        int fd = -1;
        // prepare socket
        if ((fd = socket(PF_INET, SOCK_STREAM, 0) < 0) {
            LEAVING;
            return 0;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_aton(ip.c_str(), &server_addr.sin_addr) == 0) {
            LEAVING;
            return 0;
        }
        socklen_t addr_len = sizeof(struct sockaddr_in);

        // connect
        if (connect(fd_, (struct sockaddr*)&server_addr, addr_len) == -1) {
            LEAVING;
            return 0;
        }

        // Get my_ip of this socket
        struct sockaddr_in myaddr;
        socklen_t myaddr_len;
        if (getsockname(fd_, (struct sockaddr*)&myaddr, &myaddr_len) == -1) {
            LEAVING;
            return 0;
        }

        std::string tmpip = inet_ntoa(&myaddr.sin_addr);
        Socket *ret_sk = new Socket(fd);
        ret_sk->set_my_ipstr(tmpip);
        ret_sk->set_my_ip(myaddr.sin_addr);
        ret_sk->set_my_port(ntohs(myaddr.sin_port));
        ret_sk->set_peer_ip(server_addr.sin_addr);
        ret_sk->set_peer_ipstr(ip)
        ret_sk->set_peer_port(port);
        LEAVING;
        return 0;
    };

    // 阻塞的发送
    int TCPSend(Socket *sk, char *buf_to_send,
                       uint32_t buf_to_send_size) {

        ENTERING;
        if (buf_to_send == 0) {
            LEAVING;
            return -1;
        }

        // send
        uint32_t size_left = buf_to_send_size;
        char *cur = buf_to_send;
        while (size_left) {
            int ret = send(sk->sk(), buf_to_send, buf_to_send_size, 0);
            if (ret == -1) {
                if (errno != EWOULDBLOCK || errno != EAGAIN) {
                    LEAVING;
                    return -2;
                }
            } else {
                size_left -= ret;
                cur += ret;
            }
        }
        LEAVING;
        return 0;
    };

    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) = 0;

    // 等待回应到来，因为后端服务器处理时间会比较长，因此
    // 顺序等待比较浪费，因此利用epoll
    // 返回值：
    // -3：出错
    // -2：超时
    // -1：成功（丑！）
    // >=0：指示第几个socket出错
    int TCPWaitToRead(std::vector<Socket*> &sk_list, int &num_of_triggered_fd, int millisecs) {

        ENTERING;
        uint32_t len = sk_list.size();
        assert(len >= num_of_triggered_fd);
        assert(num_of_triggered_fd > 0);

        std::vector<struct epoll_event> evs(len);
        int num = num_of_triggered_fd;
        num_of_triggered_fd = 0;

        // add fd to epoll
        for (uint32_t i = 0; i < len; i++) {

            evs[i].events = EPOLLIN|EPOLLONESHOT; // NOTE: onshot event
            evs[i].data.fd = sk_list[i]->sk();
            evs[i].data.u32 = i;
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, e.data.fd, &evs[i]) < 0) {
                // roll back
                for (uint32_t j = 0; j <= i; j++) {
                    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, evs[j].data.fd, 0);
                }
                LEAVING;
                return j;
            }
        }// for

        // wait
        int timeout = millisecs;
        while (true) {

            struct timeval start_time, end_time;
            gettimeofday(&start_time);
            int ret = epoll_wait(epoll_fd_, &evs[0], len, timeout);
            if (ret < 0) {
                if (errno != EINTR) {
                    LEAVING;
                    return -2;
                }
                gettimeofday(&end_time);
                timeout -= (end_time.tv_sec - start_time.tv_sec) * 1000
                            + (end_time.tv_usec - start_time.tv_usec) / 1000;

                if (timeout <= 0) {
                    LEAVING;
                    return -2;
                }
                continue;
            }
            num_of_triggered_fd += ret;
            if (num_of_triggered_fd == num) {
                break;
            }
        }// while
        LEAVING;
        return -1;
    };

private:
};
};// namespace NF

#endif // __CLIENT_HPP__
