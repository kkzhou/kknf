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

#include "socketio.hpp"
#include "util.hpp"
#include "socket.hpp"
#include "socketaddr.hpp"

namespace NF {

class Client: public SocketIO
{
public:
    Client()
	{
        epoll_fd_ = epoll_create(1024);
        if (epoll_fd_ < 0) {
            SLOG(2, "epoll_create() error %s\n", strerror(errno));
        }
        client_socket_idle_list_mutex_ = new pthread_mutex_t;
        pthread_mutex_init(client_socket_idle_list_mutex_, 0);
    };
    virtual ~Client(){};

public:
	// 等待回应到来，因为后端服务器处理时间会比较长，因此
    // 顺序等待比较浪费，因此利用epoll
    // 返回值：
    // -3：系统错误
    // -2：套接口出错
    // -1：超时
    // =0：成功
    int TCPWaitToRead(std::vector<Socket*> &sk_list_to_read,
                      std::vector<uint32_t> &sk_list_triggered,
                      std::vector<uint32_t> &sk_list_error,
                      uint32_t num_of_triggered_fd, int timeout_millisecs) {

        ENTERING;
        uint32_t len = sk_list_to_read.size();
        assert(len >= num_of_triggered_fd);
        assert(num_of_triggered_fd > 0);

        std::vector<struct epoll_event> evs(len);

        int ret = 0;
        // add fd to epoll
        SLOG(2, "To wait %u sockets\n", len);
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
                sk_list_error.push_back(i);
                LEAVING;
                return -2;
            }
            SLOG(2, "No.%u added\n", i);
        }// for

        // wait
        int timeout = timeout_millisecs;
        while (true) {

            SLOG(2, "Begin epoll_wait()\n");
            struct timeval start_time, end_time;
            gettimeofday(&start_time, 0);
            ret = epoll_wait(epoll_fd_, &evs[0], len, timeout);
            if (ret < 0) {
                SLOG(2, "epoll_wait() error: %s\n", strerror(errno));
                if (errno != EINTR) {
                    LEAVING;
                    return -3;
                }
                gettimeofday(&end_time, 0);
                timeout -= (end_time.tv_sec - start_time.tv_sec) * 1000
                            + (end_time.tv_usec - start_time.tv_usec) / 1000;

                if (timeout <= 10) {
                    SLOG(2, "It's timeout\n");
                    LEAVING;
                    return -1;
                }
                continue;
            }

            for (int i = 0; i < ret; i++) {

				epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, evs[i].data.fd, 0);
                if (evs[i].events & EPOLLIN) {
                    SLOG(2, "A socket ready: %d\n", i);
                    sk_list_triggered.push_back(evs[i].data.u32);
                } else if (evs[i].events & EPOLLERR) {
                    SLOG(2, "A socket error: %d\n", i);
                    sk_list_error.push_back(evs[i].data.u32);
                } else {
                    SLOG(2, "event not supported: %d\n", evs[i].events);
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

       
	// 获取空闲的TCP套接口，用于向后端服务器收发数据
    Socket* GetIdleClientSocket(std::string &to_ip, uint16_t to_port) {

        ENTERING;
        SLOG(2, "To get a client socket to <%s : %u>\n", to_ip.c_str(), to_port);
        Socket *ret_sk = this->GetClientSocket(to_ip, to_port);
        if (!ret_sk) {
            SLOG(2, "Not found to make a new one\n");
            ret_sk = this->MakeConnection(to_ip, to_port);
        }
        LEAVING;
        return ret_sk;
    };


    // 通过这个函数找一个空闲的
    // 连接，如果没有则返回0，用MakeConnection函数建立一个新的连接
    Socket* GetClientSocket(std::string &ip, uint16_t port) {

        ENTERING;
        Socket *ret_sk = 0;
        SocketAddr addr;
        addr.ip_ = ip;
        addr.port_ = port;

        SLOG(2, "To find a client socket to <%s : %u>\n", ip.c_str(), port);

        std::map<SocketAddr, std::list<Socket*> >::iterator it;
        pthread_mutex_lock(client_socket_idle_list_mutex_);
        it = client_socket_idle_list_.find(addr);

        if (it != client_socket_idle_list_.end()) {

            assert(it->second.size() != 0);
            ret_sk = it->second.front();
            it->second.pop_front();
            SLOG(2, "Found a socket <%s : %u> -> <%s : %u>\n",
                 ret_sk->peer_ipstr().c_str(),
                 ret_sk->peer_port(),
                 ret_sk->my_ipstr().c_str(),
                 ret_sk->my_port());

            if (it->second.size() == 0) {
                SLOG(2, "This is the last idle socket to <%s : %u>\n", ip.c_str(), port);
                client_socket_idle_list_.erase(it);
            }
        } else {
            SLOG(2, "Not found\n");
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
        SLOG(2, "Insert a client socket <%s : %u> -> <%s : %u>\n",
                 sk->peer_ipstr().c_str(),
                 sk->peer_port(),
                 sk->my_ipstr().c_str(),
                 sk->my_port());

        pthread_mutex_lock(client_socket_idle_list_mutex_);
        std::map<SocketAddr, std::list<Socket*> >::iterator it;

        it = client_socket_idle_list_.find(addr);

        if (it != client_socket_idle_list_.end()) {

            assert(it->second.size() > 0);
            SLOG(2, "Already exist %zd socket to <%s : %u>\n", it->second.size(), addr.ip_.c_str(), addr.port_);
            it->second.push_back(sk);

        } else {
            SLOG(2, "No socket to <%s : %u> exists\n", addr.ip_.c_str(), addr.port_);
            std::list<Socket*> new_list;
            new_list.push_back(sk);
            client_socket_idle_list_.insert(std::pair<SocketAddr, std::list<Socket*> >(addr, new_list));
        }
        pthread_mutex_unlock(client_socket_idle_list_mutex_);
        LEAVING;
        return 0;
    };

    //void set_max_udp_pkt_size(int max) { max_udp_pkt_size_ = max; };
    //void set_max_tcp_pkt_size(int max) { max_tcp_pkt_size_ = max; };
private:
    // 连接服务器的套接口，对同一个ip：port可能有多个套接口
    std::map<SocketAddr, std::list<Socket*> > client_socket_idle_list_;
    pthread_mutex_t *client_socket_idle_list_mutex_;

    int epoll_fd_;
	int max_client_socket_num_;
};
};// namespace NF

#endif // __CLIENT_HPP__
