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
class ReqFormat1 {
public:
    int l_;
    int seq_;
    int num_;
};
class ReqFormat2 {
public:
    char cmd_line_[0];
};
class ReqFormat3 {
public:
    int seq_;
    int num_;
};
class RspFormat {
public:
    int l_;
    int seq_;
    int num2_;
};
#pragma pack(0)

// 一个Processor对应一个线程
class TestSimpleProcessor1 : public Processor {

public:
    TestSimpleProcessor1(Server *srv, uint32_t pool_index)
        : Processor(srv, pool_index) {
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
            } else if (ret == 0) {
                SLOG(4, "Peer closed\n");
                LEAVING;
                return -1;
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
                continue;
            }
            // 第二步
            // 阻塞接收数据
            std::vector<char> buf;
            int ret = TCPRecv(sk, buf);
            if (ret < 0) {
                // 错误，不可恢复
                sk->Close();
                delete sk;
                continue;
            }
            // 第三步
            // 处理逻辑
            ReqFormat1 *req = reinterpret_cast<ReqFormat1*>(&buf[0]);
            req->l_ = ntohl(req->l_);
            req->num_ = ntohl(req->num_);
            req->seq_ = ntohl(req->seq_);
            cout << "Recv ReqFormat1 seq = " << req->seq_ << " num = " << req->num_ << endl;

            RspFormat rsp;
            rsp.num2_ = htonl(req->num_ + 1);
            rsp.l_ = htonl(sizeof(RspFormat));
            rsp.seq_ = htonl(req->seq_);
            cout << "Send RspFormat seq = " << rsp.seq_ << " num2_ = " << rsp.num2_ << endl;
            // 第四步
            // 返回结果
            ret = TCPSend(sk, reinterpret_cast<char*>(&rsp), sizeof(RspFormat));
            if (ret < 0) {
                cout << "Send error" << endl;
            }

            srv()->InsertServerReuseSocket(pool_index(), sk);
        } // while
        LEAVING;
        return 0;
    };
};

class TestSimpleProcessor2 : public Processor {

public:
    TestSimpleProcessor2(Server *srv, uint32_t pool_index)
        : Processor(srv, pool_index) {
    };
    //接收数据，需要根据业务定义的Packetize策略来处理
    // 本测试程序中是line格式。简单起见，不判断是否收到整行，假设都是整行，而且只提取第一行。
    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) {

        ENTERING;
        int ret = 0;
        SLOG(4, "Begin to recv a line\n");
        buf_to_fill.resize(1024);
        ret = recv(sk->sk(), &buf_to_fill[0], buf_to_fill.size(), MSG_DONTWAIT);
        if (ret < 0) {
            SLOG(4, "recv() error: %s\n", strerror(errno));
            if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK) {
                return -1;
            } else {
                return -2;
            }
        }
        SLOG(4, "Recved %d bytes\n", ret);
        buf_to_fill.resize(ret);
        LEAVING;
        return 0;
    };

    // 处理逻辑
    virtual int Process() {

        ENTERING;
        while (true) {
            // 第一步
            // 阻塞获取有数据到来的套接口
            SLOG(4, "To get a ready server socket in pool %zd\n", pool_index());
            Socket *sk = srv()->GetServerReadySocket(pool_index());
            if (!sk) {
                SLOG(4, "Get ready server socket error(cann't be here)\n");
                continue;
            }
            // 第二步
            // 阻塞接收数据
            std::vector<char> buf;
            int ret = TCPRecv(sk, buf);
            if (ret < 0) {
                // 错误，不可恢复
                SLOG(4, "TCPRecv() error\n");
                sk->Close();
                delete sk;
                continue;
            } else if (ret == -2) {
                SLOG(4, "TCPRecv() error, peer closed\n");
                sk->Close();
                delete sk;
                continue;
            } else {
                SLOG(4, "Recv OK, to process\n");
            }
            // 第三步
            // 处理逻辑
            ReqFormat2 *req = reinterpret_cast<ReqFormat2*>(&buf[0]);
            int i = 0;
            string cmdline;

            for (i = 0; i < ret; i++) {
                if (req->cmd_line_[i] == '\n') {
                    req->cmd_line_[i] = '\0';
                    cmdline.append(&req->cmd_line_[0]);
                    SLOG(4, "Line extraced: %s\n", cmdline.c_str());
                    break;
                }
            }

            if (i == ret) {
                SLOG(4, "Recved data not contains a complete line\n");
                sk->Close();
                delete sk;
                continue;
            }

            i = 0;

            SLOG(4, "cmdline = %s\n", cmdline.c_str());
            string rspline = "You sent: " + cmdline + "\n";
            SLOG(4, "Rsp is = %s\n", rspline.c_str());
            ret = TCPSend(sk, const_cast<char*>(rspline.data()), rspline.size());
            if (ret < 0) {
                SLOG(4, "TCPSend() error\n");
            }
            SLOG(4, "Return the ready server socket\n");
            srv()->InsertServerReuseSocket(pool_index(), sk);
        } // while

        LEAVING;
        return 0;
    };
};


class TestSimpleProcessor3 : public Processor {

public:
    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) {
        assert(false);
        return 0;
    };

    TestSimpleProcessor3(Server *srv, uint32_t pool_index)
        : Processor(srv, pool_index) {
    };
    // 是UDP数据
    // 处理逻辑
    virtual int Process() {

        ENTERING;
        while (true) {
            // 第一步
            // 阻塞获取有数据到来的套接口
            SLOG(4, "To get a UDP packet\n");
            Packet *pkt = srv()->GetUDPPacket();
            if (!pkt) {
                SLOG(4, "Get UDP packet error(cann't be here)\n");
                continue;
            }
            // 第二步
            // 处理逻辑
            ReqFormat3 *req = reinterpret_cast<ReqFormat3*>(&pkt->data_[0]);
            req->num_ = ntohl(req->num_);
            req->seq_ = ntohl(req->seq_);

            SLOG(4, "Recv ReqFormat3 seq = %d num = %d\n", req->seq_, req->num_);

            RspFormat rsp;
            rsp.l_ = htonl(sizeof(RspFormat));
            rsp.num2_ = htonl(req->num_ + 1);
            rsp.seq_ = htonl(req->seq_);


            SLOG(4, "To send RspFormat seq = %d num2 = %d\n", rsp.seq_, rsp.num2_);
            // 第四步
            // 返回结果
            int ret = srv()->UDPSend(pkt->from_.ip_, pkt->from_.port_, reinterpret_cast<char*>(&rsp), sizeof(RspFormat));
            if (ret < 0) {
                SLOG(4, "UDPSend() error\n");
            }
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
    string myip[3];
    uint16_t myport[3];

    for (uint16_t i = 0; i < 3; i++) {
        myip[i] = "127.0.0.1";
        myport[i] = 20031 + i;
        SLOG(4, "Add listen socket\n");
        srv->AddListenSocket(myip[i], myport[i]);
    }


    srv->InitServer();
    // epoll线程启动，即用于检测套接口的线程
    pthread_t epoll_pid;
    if (pthread_create(&epoll_pid, 0, Server::ServerThreadProc, srv) < 0) {
        SLOG(4, "Create epoll thread error\n");
        return -1;
    }
    // 启动worker线程
    pthread_t worker_pid[3][4];
    Processor *worker_processor[3][4];

    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 4; i++) {
            switch (j) {
            case 0:
                // format 1
                worker_processor[j][i] = new TestSimpleProcessor1(srv, j);
                break;
            case 1:
                worker_processor[j][i] = new TestSimpleProcessor2(srv, j);
                break;
            case 2:
                worker_processor[j][i] = new TestSimpleProcessor3(srv, j);
                break;
            default:
                SLOG(4,"Not supported format\n");
                return -1;
                break;
            }// switch


            if (pthread_create(&worker_pid[j][i], 0, Processor::ProcessorThreadProc, worker_processor[j][i]) < 0) {
                SLOG(4, "Create worker thread error\n");
                return -1;
            }
            SLOG(4, "Create worker thread pid = %lu\n", worker_pid[j][i]);
        }
    } // for

    // 等待线程完成
    pthread_join(epoll_pid, 0);
    SLOG(4, "epoll thread exits\n");

    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 4; i++) {
            pthread_join(worker_pid[j][i], 0);
            SLOG(4, "worker thread[%d][%d] exists, pid = %lu\n", j, i, worker_pid[j][i]);
        }
    }

    return 0;
}
