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

#include <map>
#include <list>
#include <assert.h>

#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "util.hpp"
#include "socket.hpp"

namespace NF {

class SocketAddr {
public:
    std::string ip_;
    uint16_t port_;
public:
    bool operator< (const SocketAddr &o) const {
        if (ip_ < o.ip_) {
            return true;
        } else if (ip_ > o.ip_) {
            return false;
        } else {
            if (port_ < o.port_) {
                return true;
            } else {
                return false;
            }
        }
    };
};

class Client {
public:
    Client() {
        epoll_fd_ = epoll_create(1024);
        client_socket_idle_list_mutex_ = new pthread_mutex_t;
        pthread_mutex_init(client_socket_idle_list_mutex_, 0);
    };
    virtual ~Client(){};

public:
    int MakeUDPSocket() {

        ENTERING;
        int fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (fd < 0) {
            LEAVING;
            return -1;
        }

        LEAVING;
        return fd;
    };
    // 阻塞的创建一个连接，然后设置成非阻塞
    Socket* MakeConnection(std::string &ip, uint16_t port) {

        ENTERING;
        int fd = -1;
        // prepare socket
        if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
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
        if (connect(fd, (struct sockaddr*)&server_addr, addr_len) == -1) {
            LEAVING;
            return 0;
        }

        // Get my_ip of this socket
        struct sockaddr_in myaddr;
        socklen_t myaddr_len;
        if (getsockname(fd, (struct sockaddr*)&myaddr, &myaddr_len) == -1) {
            LEAVING;
            return 0;
        }

        std::string tmpip = inet_ntoa(myaddr.sin_addr);
        Socket *ret_sk = new Socket(fd);
        ret_sk->set_my_ipstr(tmpip);
        ret_sk->set_my_ip(myaddr.sin_addr);
        ret_sk->set_my_port(ntohs(myaddr.sin_port));
        ret_sk->set_peer_ip(server_addr.sin_addr);
        ret_sk->set_peer_ipstr(ip);
        ret_sk->set_peer_port(port);
        LEAVING;
        return ret_sk;
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

    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) {

        ENTERING;
        LEAVING;
        return 0;
    };

    // 等待回应到来，因为后端服务器处理时间会比较长，因此
    // 顺序等待比较浪费，因此利用epoll
    // 返回值：
    // -3：系统错误
    // -2：套接口出错
    // -1：超时
    // =0：成功
    int TCPWaitToRead(std::vector<Socket*> &sk_list_to_read,
                      std::vector<Socket*> &sk_list_triggered,
                      std::vector<Socket*> &sk_list_error,
                      uint32_t num_of_triggered_fd, int timeout_millisecs) {

        ENTERING;
        uint32_t len = sk_list_to_read.size();
        assert(len >= num_of_triggered_fd);
        assert(num_of_triggered_fd > 0);

        std::vector<struct epoll_event> evs(len);

        int ret = 0;
        // add fd to epoll
        for (uint32_t i = 0; i < len; i++) {

            evs[i].events = EPOLLIN|EPOLLONESHOT; // NOTE: onshot event
            evs[i].data.fd = sk_list_to_read[i]->sk();
            evs[i].data.u32 = i;
            if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, evs[i].data.fd, &evs[i]) < 0) {
                // roll back
                SLOG(2, "epoll_ctl(add) error: %s\n", strerror(errno));
                for (uint32_t j = 0; j <= i; j++) {
                    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, evs[j].data.fd, 0);
                }
                sk_list_error.push_back(sk_list_to_read[i]);
                LEAVING;
                return -2;
            }
        }// for

        // wait
        int timeout = millisecs;
        while (true) {

            struct timeval start_time, end_time;
            gettimeofday(&start_time, 0);
            ret = epoll_wait(epoll_fd_, &evs[0], len, timeout);
            if (ret < 0) {
                if (errno != EINTR) {
                    SLOG(2, "epoll_wait error: %s\n", strerror(errno));
                    LEAVING;
                    return -3;
                }
                gettimeofday(&end_time, 0);
                timeout -= (end_time.tv_sec - start_time.tv_sec) * 1000
                            + (end_time.tv_usec - start_time.tv_usec) / 1000;

                if (timeout <= 10) {
                    SLOG(2, "timeout\n");
                    LEAVING;
                    return -1;
                }
                continue;
            }

            for (int i = 0; i < ret; i++) {
                if (evs[i].events & EPOLLEIN) {
                    sk_list_triggered.push_back([sk_list_to_read[evs[i].data.u32]);
                } else if (evs[i].events & EPOLLERR) {
                    sk_list_error.push_back([sk_list_to_read[evs[i].data.u32]);
                } else {
                    SLOG(2, "event error: %d\n", evs[i].events);
                    assert(false);
                }

            }
            uint32_t real_num = sk_list_error.size() + sk_list_triggered.size();
            if (num_of_triggered_fd <= real_num) {
                SLOG(2, "%u socket triggered\n", real_num);
                break;
            }
        }// while

        LEAVING;
        return 0;
    };

    // UDP数据发送，因为不是流，只能由应用来把请求和应答对应起来。
    int UDPSend(int sk, std::string &to_ip, uint16_t to_port, char *buf_to_send, int buf_len) {

        ENTERING;
        struct sockaddr_in to_addr;
        socklen_t addr_len;

        memset(&to_addr, sizeof(struct sockaddr_in), 0);
        to_addr.sin_port = htons(to_port);
        if (inet_aton(to_ip.c_str(), &to_addr.sin_addr) == 0) {
            LEAVING;
            return -1;
        }
        addr_len = sizeof(struct sockaddr_in);
        // udp_socket_ is BLOCKING fd
        int ret = sendto(sk, buf_to_send, buf_len, 0,
                         (struct sockaddr*)(&to_addr), addr_len);

        if (ret <= 0) {
            LEAVING;
            return -1;
        }
        LEAVING;
        return 0;
    };

    // UDP数据接收，因为不是流，只能由应用来把请求和应答对应起来。
    int UDPRecv(int sk, std::vector<char> &buf_to_fill, std::string &from_ip, uint16_t &from_port) {

        ENTERING;
        struct sockaddr_in from_addr;
        socklen_t addr_len;

        memset(&from_addr, sizeof(struct sockaddr_in), 0);
        addr_len = sizeof(struct sockaddr_in);

        buf_to_fill.resize(65535);
        // udp_socket_ is BLOCKING fd
        int ret = recvfrom(sk, &buf_to_fill[0], buf_to_fill.size(), 0,
                         (struct sockaddr*)(&from_addr), &addr_len);

        if (ret <= 0) {
            LEAVING;
            return -1;
        }

        from_ip = inet_ntoa(from_addr.sin_addr);
        from_port = ntohs(from_addr.sin_port);
        buf_to_fill.resize(ret);
        LEAVING;
        return 0;
    };

    // 通过这个函数找一个空闲的
    // 连接，如果没有则返回0，用MakeConnection函数建立一个新的连接
    Socket* GetClientSocket(std::string &ip, uint16_t port) {

        ENTERING;
        Socket *ret_sk = 0;
        SocketAddr addr;
        addr.ip_ = ip;
        addr.port_ = port;

        std::map<SocketAddr, std::list<Socket*> >::iterator it;

        pthread_mutex_lock(client_socket_idle_list_mutex_);
        it = client_socket_idle_list_.find(addr);

        if (it != client_socket_idle_list_.end()) {

            assert(it->second.size() != 0);
            ret_sk = it->second.front();
            it->second.pop_front();

            if (it->second.size() == 0) {
                client_socket_idle_list_.erase(it);
            }
        }

        pthread_mutex_unlock(client_socket_idle_list_mutex_);
        LEAVING;
        return ret_sk;
    };

    // 把新建立的连向后端服务器的连接放到列表里
    // 或者是把用过的连接交回到列表里
    int InsertClientSocket(Socket *sk) {

        ENTERING;
        SocketAddr addr;
        addr.ip_ = sk->peer_ipstr();
        addr.port_ = sk->peer_port();

        pthread_mutex_lock(client_socket_idle_list_mutex_);
        std::map<SocketAddr, std::list<Socket*> >::iterator it;

        it = client_socket_idle_list_.find(addr);

        if (it != client_socket_idle_list_.end()) {

            assert(it->second.size() > 0);
            it->second.push_back(sk);

        } else {

            std::list<Socket*> new_list;
            new_list.push_back(sk);
            client_socket_idle_list_.insert(std::pair<SocketAddr, std::list<Socket*> >(addr, new_list));
        }
        pthread_mutex_unlock(client_socket_idle_list_mutex_);
        LEAVING;
        return 0;
    };

    int max_udp_pkt_size() { return max_udp_pkt_size_;};
    int max_tcp_pkt_size() { return max_tcp_pkt_size_;};
private:
    // 连接服务器的套接口，对同一个ip：port可能有多个套接口
    std::map<SocketAddr, std::list<Socket*> > client_socket_idle_list_;
    pthread_mutex_t *client_socket_idle_list_mutex_;

    int epoll_fd_;
    int max_udp_pkt_size_;
    int max_tcp_pkt_size_;
};
};// namespace NF

#endif // __CLIENT_HPP__
