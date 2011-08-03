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
            ret = recv(sk->sk(), &len_field, 4 - byte_num, 0);
            if (ret < 0) {
                if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK) {
                    return -1;
                }
            } else {
                byte_num += ret;
                continue;
            }
        } // while

        int len = ntohl(len_field);
        if ( len <= 0 || len > max_tcp_pkt_size()) {
            LEAVING;
            return -2;
        }

        // 收数据域
        byte_num = 0;
        buf_to_fill.resize(len);
        while (byte_num < len) {
            ret = recv(sk->sk(), &buf_to_fill[0], len - byte_num, 0);
            if (ret < 0) {
                if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK) {
                    LEAVING;
                    return -2;
                }
            } else {
                byte_num += ret;
                continue;
            }
        } // while
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
    // 本测试程序中是line格式
    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) {

        ENTERING;
        int ret = 0;

        buf_to_fill.resize(1024);
        ret = recv(sk->sk(), &buf_to_fill[0], buf_to_fill.size(), MSG_DONTWAIT);
        if (ret < 0) {
            if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK) {
                return -1;
            } else {
                return 0;
            }
        }

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
            } else if (ret == 0) {
                srv()->InsertServerReuseSocket(pool_index(), sk);
                continue;
            } else {
            }
            // 第三步
            // 处理逻辑
            ReqFormat2 *req = reinterpret_cast<ReqFormat2*>(&buf[0]);
            int i = 0;
            for (i = 0; i < ret; i++) {
                if (req->cmd_line_[i] == '\n') {
                    req->cmd_line_[i] = '\0';
                }
            }
            req->cmd_line_[ret - 1] = '\0';

            i = 0;
            while (i < ret) {
                string cmd_line;
                cmd_line.append(&req->cmd_line_[i]);
                i += cmd_line.size() + 1;

                cout << "Recv ReqFormat2 cmdline = " << cmd_line << endl;
                string rsp_line = "You sent: " + cmd_line + "\n";
                ret = TCPSend(sk, const_cast<char*>(rsp_line.data()), rsp_line.size());
                if (ret < 0) {
                    cout << "Send error" << endl;
                }
            }
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
            Packet *pkt = srv()->GetUDPPacket();
            if (!pkt) {
                continue;
            }
            // 第二步
            // 处理逻辑
            ReqFormat3 *req = reinterpret_cast<ReqFormat3*>(&pkt->data_[0]);
            req->num_ = ntohl(req->num_);

            cout << "Recv ReqFormat3 num = " << req->num_ << endl;

            RspFormat rsp;
            rsp.l_ = htonl(sizeof(RspFormat));
            rsp.num2_ = htonl(req->num_ + 1);
            rsp.seq_ = htonl(req->seq_);


            cout << "Send RspFormat seq = " << rsp.seq_ << " num = " << rsp.num2_ << endl;
            // 第四步
            // 返回结果
            int ret = srv()->UDPSend(pkt->from_.ip_, pkt->from_.port_, reinterpret_cast<char*>(&rsp), sizeof(RspFormat));
            if (ret < 0) {
                cout << "Send error" << endl;
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
        cout << "Timer triggered " << __PRETTY_FUNCTION__ << endl;
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
    srv->AddListenSocket(myip1, myport1);
    srv->AddListenSocket(myip2, myport2);
    srv->AddListenSocket(myip3, myport3);

    srv->InitServer();
    // epoll线程启动，即用于检测套接口的线程
    pthread_t epoll_pid;
    if (pthread_create(&epoll_pid, 0, Server::ServerThreadProc, srv) < 0) {
        cout << "Create epoll thread error" << endl;
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
                worker_processor[j][i] = new TestSimpleProcessor1(srv, 0);
                break;
            case 1:
                worker_processor[j][i] = new TestSimpleProcessor2(srv, 0);
                break;
            case 2:
                worker_processor[j][i] = new TestSimpleProcessor3(srv, 0);
                break;
            default:
                cout << "Not supported format" << endl;
                return -1;
                break;
            }// switch


            if (pthread_create(&worker_pid[j][i], 0, Processor::ProcessorThreadProc, worker_processor[j][i]) < 0) {
                cout << "Create worker thread No." << i << " error" << endl;
                return -1;
            }
        }
    } // for

    // 等待线程完成
    pthread_join(epoll_pid, 0);
    cout << "epoll thread done" << endl;

    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 4; i++) {
            pthread_join(worker_pid[j][i], 0);
            cout << "worker thread No.[" << j << "][" << i << "] done" << endl;
        }
    }

    return 0;
}
