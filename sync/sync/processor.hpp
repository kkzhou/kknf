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

// һ��Processor��Ӧһ���߳�
class Processor {
public:
    Processor(Server *srv, uint32_t pool_index)
        :srv_(srv),
        epoll_fd_(-1),
        pool_index_(pool_index),
        cancel_(false) {

        epoll_fd_ = epoll_create(100);
        max_udp_pkt_size_ = 1472; // 1500 - 20 - 8
        max_tcp_pkt_size_ = 1024 * 1024 * 10; // 10M
    };

    virtual ~Processor() {
    };

    void Stop() {

        ENTERING;
        cancel_ = true;
        LEAVING;
    };
    // �̺߳���
    static void ProcessorThreadProc(void *arg) {

        ENTERING;
        Processor *p = reinterpret_cast<Processor>(arg);
        p->Process();
        LEAVING;
    };

    // �����߼�
    virtual int Process() = 0;

    //�������ݣ���Ҫ����ҵ�����Packetize����������
    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) = 0;


    // ��ȡ���е�TCP�׽ӿڣ��������˷������շ�����
    Socket* GetIdleClientSocket(std::string &to_ip, uint16_t to_port) {

        ENTERING;
        Socket *ret_sk = srv_->GetClientSocket(to_ip, to_port);
        if (!ret_sk) {
            ret_sk = srv_->MakeConnection(to_ip, to_port);
        }
        LEAVING;
        return ret_sk;
    };

    // �����ķ���
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

    // �ȴ���Ӧ��������Ϊ��˷���������ʱ���Ƚϳ������
    // ˳��ȴ��Ƚ��˷ѣ��������epoll
    // ����ֵ��
    // -3������
    // -2����ʱ
    // -1���ɹ����󣡣�
    // >=0��ָʾ�ڼ���socket����
    int TCPWaitToRead(std::vector<Socket*> &sk_list, int &num_of_triggered_fd, int millisecs) {

        ENTERING;
        uint32_t len = sk_list.size();
        assert(len >= num_of_triggered_fd);
        assert(num_of_triggered_fd > 0);

        std::vector<struct epoll_event> evs(len);
        int num = num_of_triggered_fd;
        num_of_triggered_fd = 0;

        int ret = 0;
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
            ret = epoll_wait(epoll_fd_, &evs[0], len, timeout);
            if (ret < 0) {
                if (errno != EINTR) {
                    perror("epoll_wait error:");
                    LEAVING;
                    return -3;
                }
                gettimeofday(&end_time);
                timeout -= (end_time.tv_sec - start_time.tv_sec) * 1000
                            + (end_time.tv_usec - start_time.tv_usec) / 1000;

                if (timeout <= 10) {
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
public:
    int max_udp_pkt_size() { return max_udp_pkt_size_;};
    int max_tcp_pkt_size() { return max_tcp_pkt_size_;};
    uint32_t pool_index() { return pool_index_; };
private:
    Server *srv_;
    int epoll_fd_;
    uint32_t pool_index_;    // ���̳߳�ʱ������
    bool cancel_;

    int max_udp_pkt_size_;
    int max_tcp_pkt_size_;

private:
    Processor(){};
    Processor(Processor&){};
    Processor& operator=(Processor&){};
};
}; // namespace NF

#endif // __PROCESSOR_HPP__

