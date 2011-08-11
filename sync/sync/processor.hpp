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

#ifndef __PROCESSOR_HPP__
#define __PROCESSOR_HPP__

#include "server.hpp"

namespace NF {

// 一个Processor对应一个线程
class Processor {
public:
    Processor(Server *srv, uint32_t pool_index)
        :srv_(srv),
        epoll_fd_(-1),
        pool_index_(pool_index),
        cancel_(false) {

        ENTERING;
        epoll_fd_ = epoll_create(100);
        if (epoll_fd_ < 0) {
            perror("Create epoll error: ");
        }
        max_udp_pkt_size_ = 1472; // 1500 - 20 - 8
        max_tcp_pkt_size_ = 1024 * 1024 * 10; // 10M
        LEAVING;
    };

    virtual ~Processor() {
        ENTERING;
        LEAVING;
    };

    void Stop() {

        ENTERING;
        cancel_ = true;
        LEAVING;
    };
    // 线程函数
    static void* ProcessorThreadProc(void *arg) {

        ENTERING;
        Processor *p = reinterpret_cast<Processor*>(arg);
        p->Process();
        LEAVING;
        return 0;
    };

    // 处理逻辑
    virtual int Process() = 0;

    //接收数据，需要根据业务定义的Packetize策略来处理
    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) = 0;


    // 获取空闲的TCP套接口，用于向后端服务器收发数据
    Socket* GetIdleClientSocket(std::string &to_ip, uint16_t to_port) {

        ENTERING;
        Socket *ret_sk = srv_->GetClientSocket(to_ip, to_port);
        if (!ret_sk) {
            ret_sk = srv_->MakeConnection(to_ip, to_port);
        }
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
        }// for

        // wait
        int timeout = timeout_millisecs;
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
                if (evs[i].events & EPOLLIN) {
                    sk_list_triggered.push_back(evs[i].data.u32);
                } else if (evs[i].events & EPOLLERR) {
                    sk_list_error.push_back(evs[i].data.u32);
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
public:
    int max_udp_pkt_size() { return max_udp_pkt_size_;};
    int max_tcp_pkt_size() { return max_tcp_pkt_size_;};
    uint32_t pool_index() { return pool_index_; };
    Server* srv() { return srv_; };
private:
    Server *srv_;
    int epoll_fd_;
    uint32_t pool_index_;    // 多线程池时的索引
    bool cancel_;

    int max_udp_pkt_size_;
    int max_tcp_pkt_size_;

private:
    Processor(){};
    Processor(Processor&){};
    Processor& operator=(Processor&){ return *this; };
};
}; // namespace NF

#endif // __PROCESSOR_HPP__

