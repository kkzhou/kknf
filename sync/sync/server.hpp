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

namespace NF {

class SocketAddr {
public:
    std::string ip_;
    uint16_t port_;
};

class Server {
public:
    // constructor/destructor
    Server(uint32_t epoll_size, uint32_t max_listen_socket_num, uint32_t max_sever_socket_num,
           uint32_t max_client_socket_num) {

        epoll_size_ = epoll_size_;
        max_listen_socket_num_ = max_listen_socket_num;
        max_sever_socket_num_ = max_sever_socket_num;
        max_client_socket_num_ = max_client_socket_num;
        epoll_fd_ = -1;
        server_socket_ready_list_mutex_ = PTHREAD_MUTEX_INITIALIZER;
        client_socket_idle_list_mutex_ = PTHREAD_MUTEX_INITIALIZER;
        server_socket_reuse_list_mutex_ = PTHREAD_MUTEX_INITIALIZER;
        server_socket_ready_list_cond_ = PTHREAD_COND_INITIALIZER;
        local_socket_pair_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    };
    ~Server() {};
    // initiate
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

    int RunServer() {


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
        return 0;
    };

    int MakeConnect(std::string &ip, uint16_t port, Socket &ret_sk) {

        int fd = -1;
        // prepare socket
        if ((fd = socket(PF_INET, SOCK_STREAM, 0) < 0) {
            return -1;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_aton(ip.c_str(), &server_addr.sin_addr) ==0) {
            return -1;
        }
        socklen_t addr_len = sizeof(struct sockaddr_in);

        // connect
        if (connect(fd_, (struct sockaddr*)&server_addr, addr_len) == -1) {
            return -2;
        }

        // Get my_ip of this socket
        struct sockaddr_in myaddr;
        socklen_t myaddr_len;
        if (getsockname(fd_, (struct sockaddr*)&myaddr, &myaddr_len) == -1) {
            return -2;
        }

        std::string tmpip = inet_ntoa(&myaddr.sin_addr);
        ret_sk.set_sk(fd);
        ret_sk.set_my_ip(tmpip);
        ret_sk.set_my_port(ntohs(myaddr.sin_port));
        ret_sk.set_peer_ip(ip);
        ret_sk.set_peer_port(port);
        ret_sk.SetNonBlock();
        return 0;
    };

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
        return 0;
    };

    int GetClientSocket(std::string &ip, uint16_t port, Socket &ret_sk) {

        SocketAddr addr;
        addr.ip_ = ip;
        addr.port_ = port;

        int ret = -1;
        std::map<SocketAddr, std::list<Socket> >::iterator it;
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
        return ret;
    };

    int InsertClientSocket(Socket &sk) {

        SocketAddr addr;
        addr.ip_ = sk.peer_ip();
        addr.port_ = sk.peer_port();

        pthread_mutex_lock(client_socket_idle_list_mutex_);
        std::map<SocketAddr, std::list<Socket> >::iterator it;
        it = client_socket_idle_list_.find(addr);

        if (it != client_socket_idle_list_.end()) {

            assert(it->second.size() > 0);
            it->second.push_back(sk);

        } else {

            std::list<Socket> new_list;
            new_list.push_back(sk);
            client_socket_idle_list_.insert(std::pair<SocketAddr, std::list<Socket> >(addr, new_list));
        }
        pthread_mutex_unlock(client_socket_idle_list_mutex_);
        return 0;
    };

private:
    uint32_t epoll_size_;
    uint32_t max_listen_socket_num_;
    uint32_t max_sever_socket_num_;
    uint32_t max_client_socket_num_;
private:
    std::vector<Socket> listen_socket_list_;
    Socket udp_socket_;

    std::vector<std::list<Socket> > server_socket_ready_list_;
    pthread_mutex_t server_socket_ready_list_mutex_;
    pthread_cond_t server_socket_ready_list_cond_;

    std::list<Socket> server_socket_reuse_list_;
    pthread_mutex_t server_socket_reuse_mutex_;

    std::map<SocketAddr, std::list<Socket> > client_socket_idle_list_;
    pthread_mutex_t client_socket_idle_list_mutex_;

private:
    int epoll_fd_;
    int local_socket_pair_[2];  // 用于本地通信，即worker线程通知主线程
    pthread_mutex_t local_socket_pair_mutex_;
};
}; // namespace NF

#endif // __SERVER_HPP__
