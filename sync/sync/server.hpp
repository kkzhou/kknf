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

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <vector>
#include <map>
#include <list>
#include <assert>

namespace NF {

class SocketAddr {
public:
    std::string ip_;
    uint16_t port_;
};

class Packet {
public:
    SocketAddr from_;
    SocketAddr to_;
    std::vector<char> data_;
    struct timeval arrive_time_;
};

class Server {
public:
    // constructor/destructor
    Server(uint32_t epoll_size, uint32_t max_sever_socket_num, uint32_t max_client_socket_num, int timer_interval) {

        epoll_size_ = epoll_size_;
        all_epoll_events_ = new struct epoll_event[epoll_size_];
        max_sever_socket_num_ = max_sever_socket_num;
        max_client_socket_num_ = max_client_socket_num;
        timer_interval_ = timer_interval;

        has_listen_socket_ = false;
        has_udp_socket_ = false;
        epoll_fd_ = -1;
        server_socket_ready_list_mutex_ = PTHREAD_MUTEX_INITIALIZER;
        client_socket_idle_list_mutex_ = PTHREAD_MUTEX_INITIALIZER;
        server_socket_reuse_list_mutex_ = PTHREAD_MUTEX_INITIALIZER;
        server_socket_ready_list_cond_ = PTHREAD_COND_INITIALIZER;
        local_socket_pair_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    };
    ~Server() {
        delete[] all_epoll_events_;
    };

    // 初始化，主要工作是：创建epoll、创建本地套接口
    int InitServer() {

        epoll_fd_ = epoll_create(epoll_size_);
        if (epoll_fd_ < 0) {
            return -1;
        }
        int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, local_socket_pair_);
        if (ret != 0) {
            return -1;
        }
        return 0;
    };

    // 启动服务器，主要工作有：
    // 1， 把侦听套接口添加epoll
    // 2， 处理定时器
    // 3， 把UDP套接口添加到epoll
    // 4， 进入epoll_wait
    int RunServer() {

        if (listen_socket_list_.size() == 0 && udp_socket_ == 0) {
            return -1;
        }

        int ret = 0;
        // add listen socket to epoll to accept
        if (listen_socket_list_.size() > 0) {
            std::vector<Socket*>::iterator listen_socket_it = listen_socket_list_.begin();
            uint32_t index = 0;
            for (; listen_socket_it; listen_socket_it++) {
                struct epoll_event e;
                e.events = EPOLL_IN;
                e.data.fd = (*listen_socket_it)->sk();
                e.data.u32 = index++;
                e.data.ptr = (*listen_socket_it);
                e.data.u64 = 0; // 0: tcp listen socket
                                // 1: tcp server socket
                                // 2: udp socket
                ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, (*listen_socket_it)->sk(), &e);
                if (ret == -1) {
                    return -2;
                }
            }
        }


        // add UDP socket
        if (udp_socket_) {
            struct epoll_event e;
            e.events = EPOLL_IN;
            e.data.fd = udp_socket_->sk();
            e.data.u32 = 0;
            e.data.u64 = 2;
            e.data.ptr = udp_socket_;
            ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, udp_socket_->sk(), &e);
            if (ret == -1) {
                return -2;
            }
        }


        int timeout = timer_interval_;
        while (true) {
            struct timeval start_time, end_time;
            gettimeofday(&start_time);

            int ret = epoll_wait(epoll_fd_, all_epoll_events_, epoll_size_, timeout);
            if (ret == 0) {
                // timeout
                timeout = TimerHandler();
                continue;
            } else if (ret == -1) {
                // error
                return -2;
            } else if (ret > 0) {
                // events
                struct epoll_event *e;
                for (int i = 0; i < ret; i++) {
                    if (0 > EpollProcess(e)) {
                        // delete this fd
                        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, e->fd, 0);
                    }
                }

            } else {
            }
            gettimeofday(&end_time);
            timeout = timeout - (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) / 1000;
            if (timeout <= 100) {
                // timer procision is 100ms
                timeout = timeout = TimerHandler();
            }
        } // while
        return 0;
    };

    // 处理有事件的套接口
    int EpollProcess(struct epoll_event &e) {

        int ret = 0;
        if (e.events | EPOLL_ERR) {
            return -1;
        } else if (e.events | EPOLL_IN) {

            if (e.u64 == 0) {
                // TCP listen socket
                struct sockaddr_in from_addr;
                socklen_t from_addr_len = 0;
                int new_fd = accept(e.fd, (struct sockaddr*)(&from_addr), &from_addr_len);
                if (new_fd == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR
                        || errno == ECONNABORTED || errno == EPERM) {
                        return 0;
                    } else if (errno == ENFILE || errno == EMFILE || errno == ENBUFS || errno == ENOMEM) {
                        return 0;
                    } else {
                        return -1;
                    }
                }
                Socket *listen_sk = (Socket*)(e.data.ptr);
                Socket *new_sk = new Socket(new_fd);
                new_sk->SetNonBlock();
                new_sk->set_my_ip(listen_sk->my_ip());
                new_sk->set_my_ipstr(listen_sk->my_ipstr());
                new_sk->set_my_port(listen_sk->my_port());
                new_sk->set_peer_ip(from_addr.sin_addr);
                char *from_addr_str = inet_ntoa(from_addr.sin_addr);
                if (from_addr_str == 0) {
                    delete new_sk;
                    return 0;
                }
                std::string tmps;
                tmps.append(from_addr_str);
                new_sk->set_peer_ipstr(tmps);
                server_socket_ready_list_[e.data.u32].push_back(new_sk);
            } else if (e.u64 == 1) {
                // TCP server socket
            } else if (e.u64 == 2) {
                // UDP socket
                static char buf_for_udp[65536]; // buf used to recv udp data
                int len = 65536;
                struct sockaddr_in from_addr;
                socklen_t from_addr_len = 0;
                int ret = recvfrom(e.fd, buf_for_udp, len, (struct sockaddr*)(&from_addr), &from_addr_len);
                if (ret < 0) {
                    if (errno == EAGAIN) {
                        return 0;
                    }
                }
                Packet *new_pkt = new Packet;
                new_pkt->data_.assign(buf_for_udp, buf_for_udp + ret);

                char *from_addr_str = inet_ntoa(from_addr.sin_addr);
                if (!from_addr_str) {
                    delete new_pkt;
                    return 0;
                }

                new_pkt->from_.ip_.append(from_addr_str);
                new_pkt->from_.port_ = ntohs(from_addr.sin_port);
                new_pkt->to_.ip_ = (Socket*)(e.data.ptr)->my_ipstr_;
                new_pkt->to_.port_ = (Socket*)(e.data.ptr)->my_port_;
                udp_recv_list_.push_back(new_pkt);
                return 0;
            } else {
                assert(false);
            }

        } // epoll_in event
    };

    int AddListenSocket(std::string &ip, uint16_t port) {

        int fd = socket(PF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            return -1;
        }
        // Bind address
        struct sockaddr_in listen_addr;
        listen_addr.sin_family = AF_INET;
        listen_addr.sin_port = htons(port);
        if (inet_aton(ip.c_str(), &listen_addr.sin_addr) == 0) {
            return -1;
        }
        socklen_t addr_len = sizeof(struct sockaddr_in);
        if (bind(fd, (struct sockaddr*)&listen_addr, addr_len) == -1) {
            return -1;
        }

        // Listen
        if (listen(fd, 1024) == -1) {
            return -1;
        }

        Socket newsk(fd);
        newsk.set_my_ip(ip);
        newsk.set_my_port(port);
        newsk.SetNonBlock();
        newsk.SetReuse();

        listen_socket_list_.push_back(newsk);
        has_listen_socket_ = true;
        return 0;
    };

    // 阻塞的创建一个连接，然后设置成非阻塞
    Socket* MakeConnect(std::string &ip, uint16_t port) {

        int fd = -1;
        // prepare socket
        if ((fd = socket(PF_INET, SOCK_STREAM, 0) < 0) {
            return 0;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_aton(ip.c_str(), &server_addr.sin_addr) == 0) {
            return 0;
        }
        socklen_t addr_len = sizeof(struct sockaddr_in);

        // connect
        if (connect(fd_, (struct sockaddr*)&server_addr, addr_len) == -1) {
            return 0;
        }

        // Get my_ip of this socket
        struct sockaddr_in myaddr;
        socklen_t myaddr_len;
        if (getsockname(fd_, (struct sockaddr*)&myaddr, &myaddr_len) == -1) {
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
        ret_sk->SetNonBlock();
        return 0;
    };

    // 添加一个UDP套接口，收发都用它
    int AddUDPSocket(std::string &ip, uint16_t port) {

        int fd = socket(PF_INET, SOCK_DATAGRAM, 0);
        if (fd < 0) {
            return -1;
        }
        // Bind address
        struct sockaddr_in listen_addr;
        listen_addr.sin_family = AF_INET;
        listen_addr.sin_port = htons(port);
        if (inet_aton(ip.c_str(), &listen_addr.sin_addr) == 0) {
            return -1;
        }
        socklen_t addr_len = sizeof(struct sockaddr_in);
        if (bind(fd, (struct sockaddr*)&listen_addr, addr_len) == -1) {
            return -1;
        }

        Socket newsk(fd);
        newsk.set_my_ip(ip);
        newsk.set_my_port(port);
        newsk.SetNonBlock();
        newsk.SetReuse();

        udp_socket_ = newsk;
        has_udp_socket_ = true;
        return 0;
    };

    // worker线程要发送数据给后端服务器的时候，通过这个函数找一个空闲的
    // 连接，如果没有则返回-1，worker线程用MakeConnection函数建立一个新的连接
    Socket* GetClientSocket(std::string &ip, uint16_t port) {

        Socket *ret_sk;
        SocketAddr addr;
        addr.ip_ = ip;
        addr.port_ = port;

        int ret = -1;
        std::map<SocketAddr, std::list<Socket*> >::iterator it;
        pthread_mutex_lock(client_socket_idle_list_mutex_);
        it = client_socket_idle_list_.find(addr);

        if (it != client_socket_idle_list_.end()) {

            assert(it->second.size() == 0);
            ret_sk = it->second.front()
            it->second.pop_front();

            if (it->second.size() == 0) {
                it->second.erase(addr);
            }
            ret = 0;
        } else {
            ret = -1;
        }

        pthread_mutex_unlock(client_socket_idle_list_mutex_);
        return ret_sk;
    };

    // worker把新建立的连向后端服务器的连接放到列表里
    // 或者是把用过的连接交回到列表里
    int InsertClientSocket(Socket *sk) {

        SocketAddr addr;
        addr.ip_ = sk->peer_ip();
        addr.port_ = sk->peer_port();

        pthread_mutex_lock(client_socket_idle_list_mutex_);
        std::map<SocketAddr, std::list<Socket*> >::iterator it;
        it = client_socket_idle_list_.find(addr);

        if (it != client_socket_idle_list_.end()) {

            assert(it->second.size() > 0);
            it->second.push_back(sk);

        } else {

            std::list<Socket> new_list;
            new_list.push_back(sk);
            client_socket_idle_list_.insert(std::pair<SocketAddr, std::list<Socket*> >(addr, new_list));
        }
        pthread_mutex_unlock(client_socket_idle_list_mutex_);
        return 0;
    };

    // 插入一个server套接口
    // 该函数主线程和worker都会调用
    int InsertServerSocket(int index, Socket *sk) {

        pthread_mutex_lock(&server_socket_list_mutex_);
        if (index >= server_socket_list_.size()) {
            pthread_mutex_unlock(&server_socket_list_mutex_);
            return -1;
        }
        server_socket_list_[index].push_back(sk);
        pthread_mutex_unlock(&server_socket_list_mutex_);
        return 0;
    };

    // 获取一个server套接口
    // 该函数由worker线程调用
    Socket* GetServerSocket(int index) {

        pthread_mutex_lock(&server_socket_list_mutex_);
        if (index >= server_socket_list_.size()) {
            pthread_mutex_unlock(&server_socket_list_mutex_);
            return 0;
        }
        Socket *sk = server_socket_list_[index].front();
        server_socket_list_[index].pop_front();
        pthread_mutex_unlock(&server_socket_list_mutex_);
        return sk;
    };
public:
    // 定时处理函数，返回值是告诉epoll_wait等待多久
    int TimerHandler() {
        return timer_interval_;
    };

private:

    uint32_t epoll_size_; // 最大可以epoll的套接口数目
    uint32_t max_sever_socket_num_;     // 从客户端过来的连接数的最大值
    uint32_t max_client_socket_num_;    // 向后端某个服务器发起的连接的最大数目

private:
    std::vector<Socket*> listen_socket_list_;    // 存放侦听套接口

    Socket *udp_socket_;                         // UDP套接口，一个足矣
    std::list<Packet*> udp_recv_list_;             // UDP套接口是在主线程里收数据，把数据交给worker线程

    std::vector<std::list<Socket*> > server_socket_ready_list_;  // 客户端连过来的套接口，有事件出现之后，主线程把
                                                                // 该套接口放到这个列表里（每个侦听套接口对应着一个列表）
                                                                // 然后用条件变量和signal配合通知worker线程
    pthread_mutex_t server_socket_ready_list_mutex_;
    pthread_cond_t server_socket_ready_list_cond_;

    // 当worker线程完成处理之后，如果是长连接，则交给主线程去epoll
    // 做法是：先放倒这个list，然后用本地套接口通知主线程
    std::list<Socket*> server_socket_reuse_list_;
    pthread_mutex_t server_socket_reuse_mutex_;

    std::map<SocketAddr, std::list<Socket*> > client_socket_idle_list_;  // 连接后端服务器的套接口，对同一个ip：port可能有多个套接口
    pthread_mutex_t client_socket_idle_list_mutex_;

private:
    int epoll_fd_;
    struct epoll_event *all_epoll_events_
    int local_socket_pair_[2];  // 用于本地通信，即worker线程通知主线程
    pthread_mutex_t local_socket_pair_mutex_;
    int timer_interval_;
};
}; // namespace NF

#endif // __SERVER_HPP__
