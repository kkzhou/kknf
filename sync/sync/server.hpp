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
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include <vector>
#include <map>
#include <list>
#include <string>
#include <cassert>

#include "util.hpp"
#include "socket.hpp"

namespace NF {

// IP：Port二元对，用作key来索引一个socket
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


// 用于存储一个数据包
class Packet {
public:
    SocketAddr from_;
    SocketAddr to_;
    std::vector<char> data_;    // 充当了智能指针的角色
    struct timeval arrive_time_;
};

class Server {
public:
    // constructor/destructor
    Server(uint32_t epoll_size, uint32_t max_server_socket_num,
           uint32_t max_client_socket_num, int timer_interval) {

        ENTERING;

        // epoll的大小，即同时连接数
        epoll_size_ = epoll_size;
        //用来在epoll_wait中存放返回的事件
        all_epoll_events_ = new epoll_event[epoll_size_];
        // 每个监听套接口能支持的最大连接数
        max_server_socket_num_ = max_server_socket_num;
        // 客户套接口（即我发起的连接），往同一个ip:port的最大数目
        max_client_socket_num_ = max_client_socket_num;
        // 定时器的触发时间
        timer_interval_ = timer_interval;

        epoll_fd_ = -1;

        // 客户套接口空闲列表的锁（其实是一个以ip:port为key的map，map的每个元素是列表）
        client_socket_idle_list_mutex_ = new pthread_mutex_t;
        pthread_mutex_init(client_socket_idle_list_mutex_, 0);
        // 服务套接口重用列表（即worker线程归还套接口到该列表）的锁
        server_socket_reuse_list_mutex_ = new pthread_mutex_t;
        pthread_mutex_init(server_socket_reuse_list_mutex_, 0);
        // UDP接收到的数据包列表的锁
        udp_recv_list_mutex_ = new pthread_mutex_t;
        pthread_mutex_init(udp_recv_list_mutex_, 0);
        // UDP接收到的数据包列表的条件变量
        udp_recv_list_cond_ = new pthread_cond_t;
        pthread_cond_init(udp_recv_list_cond_, 0);
        // UDP套接口的锁（发送时用，其实不需要，因为UDP能保证每次发送的完整性）
        udp_socket_mutex_ =  new pthread_mutex_t;
        pthread_mutex_init(udp_socket_mutex_, 0);

        epoll_cancel_ = false;
        LEAVING;
    };

    virtual ~Server() {
        ENTERING;
        delete[] all_epoll_events_;

        // 删除mutex和cond
        // 删除所有socket
        LEAVING;
    };

    // 定时处理函数，返回值是告诉epoll_wait等待多久
    virtual int TimerHandler() {
        ENTERING;
        SLOG(2, "Handle timout\n");
        LEAVING;
        return timer_interval_;
    };

    // 初始化，主要工作是：创建epoll、创建本地套接口
    int InitServer() {

        ENTERING;
        SLOG(2, "To Create epoll\n");
        epoll_fd_ = epoll_create(epoll_size_);
        if (epoll_fd_ < 0) {
            SLOG(2, "epoll_create() error %s\n", strerror(errno));
            LEAVING;
            return -1;
        }

        SLOG(2, "To Create local socket pair\n");
        int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, local_socket_pair_);
        if (ret != 0) {
            SLOG(2, "socketpair() error %s\n", strerror(errno));
            LEAVING;
            return -1;
        }

        local_socket_ = new Socket(local_socket_pair_[1]);
        local_socket_->set_id(0xFFFFFFFF);
        local_socket_->set_type(4);
        LEAVING;
        return 0;
    };

    // 停止epoll循环，不是立即有效
    void StopServer() {

        ENTERING;
        epoll_cancel_ = true;
        LEAVING;
    };

    // Server线程函数，线程有使用者创建
    static void* ServerThreadProc(void *arg) {

        ENTERING;
        Server *srv = reinterpret_cast<Server*>(arg);
        SLOG(2, "Start epoll thread\n");
        srv->RunServer();
        SLOG(2, "epoll thread exits\n");
        LEAVING;
        return 0;
    };

    // 添加一个UDP套接口，收发都用它
    int InitUDPSocket(std::string &ip, uint16_t port) {

        ENTERING;
        SLOG(2, "Init a UDP socket with local address <%s : %u>\n", ip.c_str(), port);
        int fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (fd < 0) {
            SLOG(2, "socket() error %s\n", strerror(errno));
            LEAVING;
            return -1;
        }

        // Bind address
        struct sockaddr_in listen_addr;
        listen_addr.sin_family = AF_INET;
        listen_addr.sin_port = htons(port);
        if (inet_aton(ip.c_str(), &listen_addr.sin_addr) == 0) {
            SLOG(2, "inet_aton() error\n");
            LEAVING;
            return -1;
        }
        socklen_t addr_len = sizeof(struct sockaddr_in);
        if (bind(fd, (struct sockaddr*)&listen_addr, addr_len) == -1) {
            perror("bind() error:");
            LEAVING;
            return -1;
        }

        udp_socket_ = new Socket(fd);
        udp_socket_->set_my_ipstr(ip);
        udp_socket_->set_my_ip(listen_addr.sin_addr);
        udp_socket_->set_my_port(port);
        udp_socket_->SetReuse();
        udp_socket_->set_id(0xFFFFFFFF);
        udp_socket_->set_type(3);
        LEAVING;
        return 0;
    };

    int AddListenSocket(std::string &ip, uint16_t port) {

        ENTERING;
        SLOG(2, "Add listen socket on <%s : %u>\n", ip.c_str(), port);
        int fd = socket(PF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            SLOG(2, "socket() error %s\n", strerror(errno));
            LEAVING;
            return -1;
        }
        // Bind address
        struct sockaddr_in listen_addr;
        listen_addr.sin_family = AF_INET;
        listen_addr.sin_port = htons(port);
        if (inet_aton(ip.c_str(), &listen_addr.sin_addr) == 0) {
            SLOG(2, "inet_aton() error\n");
            LEAVING;
            return -1;
        }

        socklen_t addr_len = sizeof(struct sockaddr_in);
        if (bind(fd, (struct sockaddr*)&listen_addr, addr_len) == -1) {
            SLOG(2, "bind() error %s\n", strerror(errno));
            LEAVING;
            return -1;
        }

        // Listen
        if (listen(fd, 1024) == -1) {
            SLOG(2, "listen() error %s\n", strerror(errno));
            LEAVING;
            return -1;
        }

        Socket *newsk = new Socket(fd);
        newsk->set_my_ipstr(ip);
        newsk->set_my_ip(listen_addr.sin_addr);
        newsk->set_my_port(port);
        newsk->SetNonBlock();   // accept4() is supported after 2.6.28
        newsk->SetReuse();
        newsk->set_id(listen_socket_list_.size());
        newsk->set_type(1);

        listen_socket_list_.push_back(newsk);
        // 因为每个listen套接口都有一个队列，因此要创建mutex和cond
        // init mutex and cond
        pthread_mutex_t *new_mutex = new pthread_mutex_t;
        pthread_mutex_init(new_mutex, 0);
        pthread_cond_t *new_cond = new pthread_cond_t;
        pthread_cond_init(new_cond, 0);

        server_socket_ready_list_mutex_.push_back(new_mutex);
        server_socket_ready_list_cond_.push_back(new_cond);
        server_socket_ready_list_.push_back(std::list<Socket*>());
        SLOG(2, "There are %zd server_socket_ready_list(s)\n", server_socket_ready_list_.size());
        int index = static_cast<int>(server_socket_ready_list_.size()) - 1;
        assert(index >= 0);
        LEAVING;
        return index;
    };

    // 阻塞的创建一个连接
    Socket* MakeConnection(std::string &ip, uint16_t port) {

        ENTERING;
        SLOG(2, "Make connection to <%s : %u>\n", ip.c_str(), port);
        int fd = -1;
        // prepare socket
        if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            SLOG(2, "socket() error %s\n", strerror(errno));
            LEAVING;
            return 0;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_aton(ip.c_str(), &server_addr.sin_addr) == 0) {
            SLOG(2, "inet_aton() error\n");
            LEAVING;
            return 0;
        }
        socklen_t addr_len = sizeof(struct sockaddr_in);

        // connect
        if (connect(fd, (struct sockaddr*)&server_addr, addr_len) == -1) {
            SLOG(2, "connect() error %s\n", strerror(errno));
            LEAVING;
            return 0;
        }

        // Get my_ip of this socket
        struct sockaddr_in myaddr;
        socklen_t myaddr_len;
        if (getsockname(fd, (struct sockaddr*)&myaddr, &myaddr_len) == -1) {
            SLOG(2, "getsockname() error %s\n", strerror(errno));
            LEAVING;
            return 0;
        }

        std::string tmpip;
        tmpip.append(inet_ntoa(myaddr.sin_addr));
        Socket *ret_sk = new Socket(fd);
        ret_sk->set_my_ipstr(tmpip);
        ret_sk->set_my_ip(myaddr.sin_addr);
        ret_sk->set_my_port(ntohs(myaddr.sin_port));
        ret_sk->set_peer_ip(server_addr.sin_addr);
        ret_sk->set_peer_ipstr(ip);
        ret_sk->set_peer_port(port);
        ret_sk->set_id(0xFFFFFFFF);
        ret_sk->set_type(5);
        SLOG(2, "Made a socket <%s : %u> -> <%s : %u>\n",
             ret_sk->peer_ipstr().c_str(),
             ret_sk->peer_port(),
             ret_sk->my_ipstr().c_str(),
             ret_sk->my_port());
        LEAVING;
        return ret_sk;
    };


    // worker线程要发送数据给后端服务器的时候，通过这个函数找一个空闲的
    // 连接，如果没有则返回-1，worker线程用MakeConnection函数建立一个新的连接
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

    // worker把新建立的连向后端服务器的连接放到列表里
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

    // 插入一个server套接口
    // 该函数主线程会调用
    int InsertServerReadySocket(std::map<uint32_t, std::list<Socket*> > &sk_map) {

        ENTERING;

        std::map<uint32_t, std::list<Socket*> >::iterator it, endit;
        it = sk_map.begin();
        endit = sk_map.end();

        while (it != endit) {
            uint32_t index = it->first;

            if (index >= server_socket_ready_list_.size()) {
                assert(false);
            }

            pthread_mutex_lock(server_socket_ready_list_mutex_[index]);

            server_socket_ready_list_[index].insert(server_socket_ready_list_[index].end(),
                                                    it->second.begin(), it->second.end());

            SLOG(2, "Inserted %zd ready server sockets for <%s : %u> -> <%s : %u> with index=%u\n",
                 it->second.size(),
                 (*it->second.begin())->peer_ipstr().c_str(),
                 (*it->second.begin())->peer_port(),
                 (*it->second.begin())->my_ipstr().c_str(),
                 (*it->second.begin())->my_port(),
                 index);

            pthread_mutex_unlock(server_socket_ready_list_mutex_[index]);
            pthread_cond_signal(server_socket_ready_list_cond_[index]);
            it++;
        }

        LEAVING;
        return 0;
    };

    // 获取一个server套接口
    // 该函数由worker线程调用
    Socket* GetServerReadySocket(uint32_t index) {

        ENTERING;
        SLOG(2, "To get a server ready socket in index = %zd\n", index);
        if (index >= server_socket_ready_list_.size()) {
            SLOG(2, "index invalid: index = %zd but server_socket_ready_list_'s size = %zd\n",
                 index, server_socket_ready_list_.size());
            LEAVING;
            return 0;
        }

        pthread_mutex_lock(server_socket_ready_list_mutex_[index]);

        while (server_socket_ready_list_[index].size() == 0) {
            pthread_cond_wait(server_socket_ready_list_cond_[index], server_socket_ready_list_mutex_[index]);
        }
        Socket *sk = server_socket_ready_list_[index].front();
        server_socket_ready_list_[index].pop_front();
        pthread_mutex_unlock(server_socket_ready_list_mutex_[index]);
        SLOG(2, "Got a ready server socket <%s : %u> -> <%s : %u>\n",
             sk->peer_ipstr().c_str(),
             sk->peer_port(),
             sk->my_ipstr().c_str(),
             sk->my_port());
        LEAVING;
        return sk;
    };

    // worker线程归还一个server套接口，并通知主线程
    // 返回值：
    // 1: 正确
    // <=0： 错误
    int InsertServerReuseSocket(uint32_t pool_index, Socket *sk) {

        ENTERING;
        int ret = 0;
        SLOG(2, "Insert a server socket to reuse <%s : %u> -> <%s : %u>\n",
             sk->peer_ipstr().c_str(),
             sk->peer_port(),
             sk->my_ipstr().c_str(),
             sk->my_port());

        pthread_mutex_lock(server_socket_reuse_list_mutex_);
        server_socket_reuse_list_[pool_index].push_back(sk);
        char notify_char = 'N';
        ret = send(local_socket_pair_[0], &notify_char, 1, 0);
        if (ret <= 0) {
            SLOG(2, "Send 1 char to local socket error: %d(%s)\n", ret, strerror(errno));
        }
        pthread_mutex_unlock(server_socket_reuse_list_mutex_);
        LEAVING;
        return ret;
    };

    // 获取所有的reusesocket，以便放到epoll
    void GetServerReuseSocketList(std::map<uint32_t, std::list<Socket*> > &sk_list) {

        ENTERING;
        assert(sk_list.size() == 0);
        pthread_mutex_lock(server_socket_reuse_list_mutex_);
        server_socket_reuse_list_.swap(sk_list);
        pthread_mutex_unlock(server_socket_reuse_list_mutex_);
        LEAVING;
        SLOG(2, "Got %zd server sockets to reuse\n", sk_list.size());
        return;
    };

    // 插入接收到的UDP数据，并通知worker
    int InsertUDPRecvPacket(std::list<Packet*> &pkt_list) {

        ENTERING;
        SLOG(2, "Insert %zd packets to list\n", pkt_list.size());
        pthread_mutex_lock(udp_recv_list_mutex_);
        udp_recv_list_.insert(udp_recv_list_.end(), pkt_list.begin(), pkt_list.end());
        pthread_mutex_unlock(udp_recv_list_mutex_);
        pthread_cond_signal(udp_recv_list_cond_);
        LEAVING;
        return 0;
    };

    Packet* GetUDPPacket() {

        ENTERING;
        Packet *ret_pkt = 0;
        pthread_mutex_lock(udp_recv_list_mutex_);
        while (udp_recv_list_.size() == 0) {
            pthread_cond_wait(udp_recv_list_cond_, udp_recv_list_mutex_);
        }

        ret_pkt = udp_recv_list_.front();
        udp_recv_list_.pop_front();
        pthread_mutex_unlock(udp_recv_list_mutex_);
        LEAVING;
        return ret_pkt;
    };

    // UDP数据发送，因为不是流，只能由应用来把请求和应答对应起来。
    int UDPSend(std::string &to_ip, uint16_t to_port, char *buf_to_send, int buf_len) {

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

        pthread_mutex_lock(udp_socket_mutex_);
        // udp_socket_ is BLOCKING fd
        int ret = sendto(udp_socket_->sk(), buf_to_send, buf_len, 0,
                         (struct sockaddr*)(&to_addr), addr_len);
        pthread_mutex_unlock(udp_socket_mutex_);
        if (ret < 0) {
            LEAVING;
            return -1;
        }
        LEAVING;
        return 0;
    };


private:
    // 启动服务器，主要工作有：
    // 1， 把侦听套接口添加epoll
    // 2， 把本地套接口加入到epoll
    // 3， 把UDP套接口添加到epoll
    // 4， 进入epoll_wait
    int RunServer() {

        ENTERING;
        if (listen_socket_list_.size() == 0 && udp_socket_ == 0) {
            SLOG(2, "No socket to listen and no udp socket to recv\n");
            LEAVING;
            return -1;
        }

        int ret = 0;
        struct epoll_event e;
        // add listen socket to epoll to accept
        if (listen_socket_list_.size() > 0) {
            std::vector<Socket*>::iterator listen_socket_it = listen_socket_list_.begin();
            std::vector<Socket*>::iterator listen_socket_endit = listen_socket_list_.end();

            for (; listen_socket_it != listen_socket_endit; listen_socket_it++) {
                memset(&e, sizeof(struct epoll_event), 0);
                e.events = EPOLLIN;
                e.data.ptr = (*listen_socket_it);
                SLOG(2, "To add listen socket into epoll, fd=%d\n", (*listen_socket_it)->sk());
                ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, (*listen_socket_it)->sk(), &e);
                if (ret == -1) {
                    perror("epoll_ctl error:");
                    LEAVING;
                    return -2;
                }
            }// for
        }

        // add UDP socket
        if (udp_socket_) {

            memset(&e, sizeof(struct epoll_event), 0);
            e.events = EPOLLIN;
            e.data.ptr = udp_socket_;
            SLOG(2, "To add UDP socket into epoll, fd=%d\n", udp_socket_->sk());
            ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, udp_socket_->sk(), &e);

            if (ret == -1) {
                perror("epoll_ctl error:");
                LEAVING;
                return -2;
            }
        }

        // add local socket
        memset(&e, sizeof(struct epoll_event), 0);
        e.events = EPOLLIN;
        e.data.ptr = local_socket_;
        SLOG(2, "To add local socket into epoll, fd=%d\n", local_socket_->sk());
        ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, local_socket_->sk(), &e);

        if (ret == -1) {

            perror("epoll_ctl error:");
            LEAVING;
            return -2;
        }

        // loop
        EpollLoop();
        LEAVING;
        return 0;
    };

    // epool 循环的线程函数
    void EpollLoop() {

        ENTERING;
        // epoll loop
        int timeout = timer_interval_;

        while (!epoll_cancel_) {

            struct timeval start_time, end_time;
            gettimeofday(&start_time, 0);

            int ret = epoll_wait(epoll_fd_, all_epoll_events_, epoll_size_, timeout);
            SLOG(2, "Epoll wake up with ret=%d\n", ret);
            if (ret == 0) {
                // timeout
                SLOG(2, "Timer fired\n");
                timeout = TimerHandler();
                continue;
            } else if (ret == -1) {
                // error
                perror("epoll_wait error:");
                if (errno == EINTR) {
                    continue;
                }
                LEAVING;
                return;
            } else if (ret > 0) {
                // events
                std::map<uint32_t, std::list<Socket*> > server_sk_map;
                for (int i = 0; i < ret; i++) {
                    Socket *triggered_sk = reinterpret_cast<Socket*>(all_epoll_events_[i].data.ptr);
                    int result = EpollProcess(all_epoll_events_[i], server_sk_map);
                    if (result < 0 || result == 1) {
                        // delete this fd
                        SLOG(2, "To delete server socket, fd=%d <%s : %u> -> <%s : %u>\n",
                             triggered_sk->sk(),
                             triggered_sk->peer_ipstr().c_str(),
                             triggered_sk->peer_port(),
                             triggered_sk->my_ipstr().c_str(),
                             triggered_sk->my_port());

                        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, triggered_sk->sk(), 0);

                        if (result < 0) {
                            SLOG(2, "Socket error, to destroy\n");
                            triggered_sk->Close();
                            all_epoll_events_[i].data.ptr = 0;
                            delete triggered_sk;
                        }

                    }
                } // for

                if (server_sk_map.size() > 0) {
                    SLOG(2, "There are some server sockets ready!\n");
                    InsertServerReadySocket(server_sk_map);
                }


            } else {
            }

            gettimeofday(&end_time, 0);
            timeout = timeout - (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) / 1000;
            if (timeout <= 100) {
                // timer procision is 100ms
                SLOG(2, "Timer fired(not exactly)\n");
                timeout = TimerHandler();
            }
        } // while

        LEAVING;
        return;
    };

    // 处理有事件的套接口
    // 用sk_map返回已经准备好的server套接口，即in/out参数（丑！）
    // 返回值：
    // <0: 出现错误，把该fd从epoll中删除
    // =0: 不做处理
    // =1: 是server套接口，已经交给了worker线程，把它从epoll中删除
    int EpollProcess(struct epoll_event &e, std::map<uint32_t, std::list<Socket*> > &server_sk_map) {

        ENTERING;
        Socket *triggered_sk = reinterpret_cast<Socket*>(e.data.ptr);
        SLOG(2, "Socket triggered <%s : %u> -> <%s : %u>\n",
                 triggered_sk->peer_ipstr().c_str(),
                 triggered_sk->peer_port(),
                 triggered_sk->my_ipstr().c_str(),
                 triggered_sk->my_port());

        if (e.events & EPOLLERR) {
            SLOG(2, "Error happends\n");
            LEAVING;
            return -1;

        } else if (e.events & EPOLLIN || e.events & EPOLLRDHUP) {

            if (triggered_sk->type() == 1) {
                // TCP listen socket
                SLOG(2, "TCP listen socket triggered\n");
                while (true) {

                    struct sockaddr_in from_addr;
                    socklen_t from_addr_len = sizeof(struct sockaddr_in);
                    int new_fd = accept(triggered_sk->sk(),
                                        (struct sockaddr*)(&from_addr), &from_addr_len);
                    if (new_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR
                            || errno == ECONNABORTED || errno == EPERM) {
                            break;
                        } else if (errno == ENFILE || errno == EMFILE || errno == ENOBUFS || errno == ENOMEM) {
                            break;
                        } else {
                            break;
                        }
                    }
                    Socket *listen_sk = reinterpret_cast<Socket*>(e.data.ptr);
                    Socket *new_sk = new Socket(new_fd);
                    new_sk->set_my_ip(listen_sk->my_ip());
                    new_sk->set_my_ipstr(listen_sk->my_ipstr());
                    new_sk->set_my_port(listen_sk->my_port());
                    new_sk->set_peer_ip(from_addr.sin_addr);
                    new_sk->set_peer_port(htons(from_addr.sin_port));
                    new_sk->set_id(listen_sk->id());
                    new_sk->set_type(2);

                    char *from_addr_str = inet_ntoa(from_addr.sin_addr);
                    if (from_addr_str == 0) {
                        SLOG(2, "inet_ntoa() error\n");
                        delete new_sk;
                        continue;
                    }
                    std::string tmps;
                    tmps.append(from_addr_str);
                    new_sk->set_peer_ipstr(tmps);

                    SLOG(2, "Create a new server socket <%s : %u> -> <%s : %u>\n",
                         new_sk->peer_ipstr().c_str(),
                         new_sk->peer_port(),
                         new_sk->my_ipstr().c_str(),
                         new_sk->my_port());

                    uint32_t index = new_sk->id();

                    // sk_map是用来装返回数据的！
                    // 如果index不存在，map会自动创建新元素，即一个空list
                    server_sk_map[index].push_back(new_sk);
                } // while
                LEAVING;
                return 0;

            } else if (triggered_sk->type() == 2) {
                // TCP server socket
                if (e.events & EPOLLRDHUP) {
                    SLOG(2, "TCP server socket closed by peer\n");
                    return -2;
                }

                SLOG(2, "TCP server socket triggered\n");
                uint32_t index = triggered_sk->id();
                server_sk_map[index].push_back(triggered_sk);
                LEAVING;
                return 1;

            } else if (triggered_sk->type() == 3) {
                // UDP socket
                SLOG(2, "UDP socket triggered\n");
                std::vector<char> buf_for_udp(65536); // buf used to recv udp data
                int len = 65536;

                std::list<Packet*> new_pkt_list;
                while (true) {

                    struct sockaddr_in from_addr;
                    socklen_t from_addr_len = 0;
                    int ret = recvfrom(triggered_sk->sk(), &buf_for_udp[0], len, MSG_DONTWAIT,
                                       (struct sockaddr*)(&from_addr), &from_addr_len);
                    if (ret <= 0) {
                        perror("recvfrom error:");
                        break;
                    }

                    Packet *new_pkt = new Packet;
                    new_pkt->data_.assign(buf_for_udp.data(), buf_for_udp.data() + ret);

                    char *from_addr_str = inet_ntoa(from_addr.sin_addr);
                    if (!from_addr_str) {
                        delete new_pkt;
                        break;
                    }

                    new_pkt->from_.ip_.append(from_addr_str);
                    new_pkt->from_.port_ = ntohs(from_addr.sin_port);
                    new_pkt->to_.ip_ = reinterpret_cast<Socket*>(e.data.ptr)->my_ipstr();
                    new_pkt->to_.port_ = reinterpret_cast<Socket*>(e.data.ptr)->my_port();
                    new_pkt_list.push_back(new_pkt);
                }

                SLOG(2, "%zd packets recved\n", new_pkt_list.size());

                if (new_pkt_list.size() > 0) {
                    InsertUDPRecvPacket(new_pkt_list);
                }

                LEAVING;
                return 0;

            } else if (triggered_sk->type() == 4) {
                // local socket
                SLOG(2, "Local socket triggered\n");
                char local_data[1024];
                while (true) {
                    int ret = recv(local_socket_pair_[1], local_data, 1024, 0);
                    if (ret <= 0) {
                        if (errno != EAGAIN || errno != EWOULDBLOCK || errno != EINTR) {
                            // fatal
                            perror("local socket error: ");
                            LEAVING;
                            exit(1);
                        }
                        break;
                    }
                    SLOG(2, "%u byte read\n", ret);
                    if (ret == 1024) {
                        // read
                        continue;
                    } else {
                        break;
                    }
                }

                // deal with server_socket_reuse_list
                std::map<uint32_t, std::list<Socket*> > sk_list;
                GetServerReuseSocketList(sk_list);

                std::map<uint32_t, std::list<Socket*> >::iterator it, endit;
                it = sk_list.begin();
                endit = sk_list.end();

                for (; it != endit; it++) {
                    std::list<Socket*>::iterator list_it, list_endit;
                    list_it = it->second.begin();
                    list_endit = it->second.end();

                    for (; list_it != list_endit; list_it++) {
                        struct epoll_event new_ev;
                        memset(&new_ev, sizeof(struct epoll_event), 0);
                        new_ev.events = EPOLLIN | EPOLLRDHUP;
                        new_ev.data.ptr = (*list_it);

                        SLOG(2, "To add reuse socket, fd=%d\n", (*list_it)->sk());
                        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, (*list_it)->sk(), &new_ev) == -1) {
                            perror("add server reuse socket error");
                            (*list_it)->Close();
                            delete (*list_it);
                            *list_it = 0;
                        } else {
                            *list_it = 0;
                        }
                    }// for
                }// for

            } else {
                assert(false);
            }

        } // epoll_in event
        LEAVING;
        return 0;
    };



private:
    // 最大可以epoll的套接口数目
    uint32_t epoll_size_;
    // 从客户端过来的连接数的最大值
    uint32_t max_server_socket_num_;
    // 向后端某个服务器发起的连接的最大数目
    uint32_t max_client_socket_num_;

private:
    // 存放侦听套接口
    std::vector<Socket*> listen_socket_list_;

    // UDP套接口，一个足矣
    // UDP套接口是在主线程里收数据，把数据交给worker线程
    Socket *udp_socket_;
    std::list<Packet*> udp_recv_list_;
    pthread_mutex_t *udp_socket_mutex_;
    pthread_mutex_t *udp_recv_list_mutex_;
    pthread_cond_t *udp_recv_list_cond_;

    // 客户端连过来的套接口，有事件出现之后，主线程把
    // 该套接口放到这个列表里（每个侦听套接口对应着一个列表）
    // 然后用条件变量和signal配合通知worker线程
    std::vector<std::list<Socket*> > server_socket_ready_list_;
    std::vector<pthread_mutex_t*> server_socket_ready_list_mutex_;
    std::vector<pthread_cond_t*> server_socket_ready_list_cond_;

    // 当worker线程完成处理之后，如果是长连接，则交给主线程去epoll
    // 做法是：先放倒这个list，然后用本地套接口通知主线程
    std::map<uint32_t, std::list<Socket*> > server_socket_reuse_list_;
    pthread_mutex_t *server_socket_reuse_list_mutex_;

    // 连接后端服务器的套接口，对同一个ip：port可能有多个套接口
    std::map<SocketAddr, std::list<Socket*> > client_socket_idle_list_;
    pthread_mutex_t *client_socket_idle_list_mutex_;

private:
    int epoll_fd_;
    struct epoll_event *all_epoll_events_;
    // 用于本地通信，即worker线程通知主线程
    int local_socket_pair_[2];
    Socket *local_socket_;  // local socket for receiving
    int timer_interval_;
private:
    // 用于控制主线程，即epoll_wait所在线程
    bool epoll_cancel_;
};
}; // namespace NF

#endif // __SERVER_HPP__
