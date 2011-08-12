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

#include "server.hpp"
#include "processor.hpp"
#include <iostream>

using namespace std;
using namespace NF;

#pragma pack(1)
class Req{
public:
    int l_;
    int seq_;
    int num_;
    int pad_[1000];
};
class Rsp{
public:
    int l_;
    int seq_;
    int num2_;
    int pad_[1000];
};
#pragma pack(0)

// 一个Processor对应一个线程
class TestSimpleProcessor : public Processor {

public:
    TestSimpleProcessor(Server *srv, uint32_t pool_index)
        : Processor(srv, pool_index) {
        ENTERING;
        LEAVING;
    };
    //接收数据，需要根据业务定义的Packetize策略来处理
    // 本测试程序中是LV格式
    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) {

        ENTERING;
        int len_field = 0;
        int byte_num = 0;
        int ret = 0;

        // 收长度域，4字节
        while (byte_num < 4) {
            SLOG(2, "Begin to recv len field\n");
            ret = recv(sk->sk(), &len_field, 4 - byte_num, 0);
            if (ret < 0) {
                if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK) {
                    SLOG(4, "recv() len field error: %s\n", strerror(errno));
                    LEAVING;
                    return -1;
                }
            } else {
                byte_num += ret;
                SLOG(2, "Recved %d bytes\n", byte_num);
                continue;
            }
        } // while

        int len = ntohl(len_field);
        SLOG(2, "The len is %d bytes\n", len);
        if ( len <= 0 || len > max_tcp_pkt_size()) {
            SLOG(4, "len field error: %d\n", len);
            LEAVING;
            return -2;
        }

        // 收数据域
        byte_num = 4;
        buf_to_fill.resize(len);
        SLOG(2, "Begin to recv %d bytes date field\n", len);
        while (byte_num < len) {
            ret = recv(sk->sk(), &buf_to_fill[0], len - byte_num, 0);
            if (ret < 0) {
                SLOG(2, "Recv returns %d, error: %s\n", ret, strerror(errno));
                if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK) {
                    SLOG(2, "recv() data field error: %s\n", strerror(errno));
                    LEAVING;
                    return -2;
                }
            } else if (ret == 0) {
                SLOG(2, "Closed by peer\n");
                LEAVING;
                return -2;
            } else {
                byte_num += ret;
                SLOG(2, "%d bytes have been recved\n", byte_num);
                continue;
            }
        } // while
        SLOG(4, "%d bytes recved\n", byte_num);
        LEAVING;
        return 0;
    };

    // 处理逻辑
    virtual int Process() {

        ENTERING;
        while (true) {
            // 第一步
            // 阻塞获取有数据到来的套接口
            Socket *sk = srv()->GetServerReadySocket(pool_index());
            if (!sk) {
                SLOG(2, "GetServerReadySocket() error\n");
                exit(1);
            }
            // 第二步
            // 阻塞接收数据
            std::vector<char> buf;
            int ret = TCPRecv(sk, buf);
            if (ret < 0) {
                // 错误，不可恢复
                SLOG(4, "TCPRecv() error, destroy this socket\n");
                sk->Close();
                delete sk;
                continue;
            }
            // 第三步
            // 处理逻辑
            Req *req = reinterpret_cast<Req*>(&buf[0]);
            req->l_ = ntohl(req->l_);
            req->num_ = ntohl(req->num_);
            req->seq_ = ntohl(req->seq_);
            SLOG(4, "Recv Req seq = %d, num = %d\n", req->seq_, req->num_);

            Rsp rsp;
            rsp.num2_ = htonl(req->num_ + 1);
            rsp.l_ = htonl(sizeof(Rsp));
            rsp.seq_ = htonl(req->seq_);
            SLOG(4, "Send Rsp seq = %d, num2 = %d\n", ntohl(rsp.seq_), ntohl(rsp.num2_));
            // 第四步
            // 返回结果
            ret = TCPSend(sk, reinterpret_cast<char*>(&rsp), sizeof(Rsp));
            if (ret < 0) {
                SLOG(4, "Send response error: %d\n", ret);
            }

            srv()->InsertServerReuseSocket(pool_index(), sk);
        } // while
        LEAVING;
        return 0;
    };
};

class TestSimpleServer : public Server {
public:
    TestSimpleServer(uint32_t epoll_size, uint32_t max_server_socket_num,
           uint32_t max_client_socket_num, int timer_interval)
           :Server(epoll_size, max_server_socket_num, max_client_socket_num, timer_interval) {

        ENTERING;
        LEAVING;
    };
    virtual int TimerHandler() {

        ENTERING;
        SLOG(4, "Timer triggered\n");
        LEAVING;
        return Server::TimerHandler();
    };
};

int main(int argc, char **argv) {

    Server *srv = new TestSimpleServer(1024, 1024, 100, 10000);
    string myip1 = "127.0.0.1";
    uint16_t myport1 = 20031;
    string myip2 = "127.0.0.1";
    uint16_t myport2 = 20032;
    string myip3 = "127.0.0.1";
    uint16_t myport3 = 20033;

    int ret = srv->AddListenSocket(myip1, myport1);
    if (ret < 0) {
        SLOG(4, "AddListenSocket() error: ip=%s port=%u\n", myip1.c_str(), myport1);
        return -1;
    }
    ret = srv->AddListenSocket(myip2, myport2);
    if (ret < 0) {
        SLOG(4, "AddListenSocket() error: ip=%s port=%u\n", myip2.c_str(), myport2);
        return -1;
    }
    ret = srv->AddListenSocket(myip3, myport3);
    if (ret < 0) {
        SLOG(4, "AddListenSocket() error: ip=%s port=%u\n", myip3.c_str(), myport3);
        return -1;
    }

    srv->InitServer();
    // epoll线程启动，即用于检测套接口的线程
    pthread_t epoll_pid;
    if (pthread_create(&epoll_pid, 0, Server::ServerThreadProc, srv) < 0) {
        SLOG(4, "Create thread for epoll error\n");
        return -1;
    }
    // 启动worker线程
    pthread_t worker_pid[4];
    int num = 4;

    for (int i = 0; i < num; i++) {
        Processor *worker_processor;

        worker_processor = new TestSimpleProcessor(srv, 0);

        if (pthread_create(&worker_pid[i], 0, Processor::ProcessorThreadProc, worker_processor) < 0) {
            SLOG(4, "Create thread for worker error");
            return -1;
        }

        SLOG(4, "No%d worker thread started\n", i);
    }

    // 等待线程完成
    pthread_join(epoll_pid, 0);
    SLOG(4, "Thread for epoll exit\n");
    for (int i = 0; i < num; i++) {
        pthread_join(worker_pid[i], 0);
        SLOG(4, "No%d worker thread exited\n", i);
    }


    return 0;
}
