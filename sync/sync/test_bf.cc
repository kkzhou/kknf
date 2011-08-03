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
#include <string>
#include <iostream>

using namespace std;
using namespace NF;

#pragma pack(1)
class ReqBB2 {
public:
    int l_;
    int seq_;
    int b_;
};
class RspBB2 {
public:
    int l_;
    int seq_;
    int b2_;
};
class ReqBB1 {
public:
    int l_;
    int seq_;
    int a_;
};
class RspBB1 {
public:
    int l_;
    int seq_;
    int a2_;
};
class ReqBF {
public:
    int l_;
    int seq_;
    int a_;
    int b_;
};
class RspBF {
public:
    int l_;
    int seq_;
    int sum_;
};
#pragma pack(0)

// 一个Processor对应一个线程
class TestBFProcessor : public Processor {

public:
    TestBFProcessor(Server *srv, uint32_t pool_index)
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
            ReqBF *req = reinterpret_cast<ReqBF*>(&buf[0]);
            req->l_ = ntohl(req->l_);
            req->a_ = ntohl(req->a_);
            req->b_ = ntohl(req->b_);
            req->seq_ = ntohl(req->seq_);

            cout << "Recv ReqBF: seq = " << req->seq_ << " a = " << req->a_ << " b = " << req->b_ << endl;

            // 1，发送请求给BB1，以得到a2
            std::string bb1_ip = "127.0.0.1";
            uint16_t bb1_port = 30021;
            Socket *bb1_sk = srv()->GetClientSocket(bb1_ip, bb1_port);
            if (!bb1_sk) {
                bb1_sk = srv()->MakeConnection(bb1_ip, bb1_port);
                if (!bb1_sk) {
                    cout << "Cant get connection: <" << bb1_ip << ", " << bb1_port << ">" << endl;
                    srv()->InsertServerReuseSocket(pool_index(), sk);
                    continue;
                }
            }

            ReqBB1 req1;
            req1.l_ = htonl(sizeof(ReqBB1));
            req1.a_ = htonl(req->a_);
            req1.seq_ = htonl(req->seq_);
            cout << "Send ReqBB1: seq = " << req1.seq_ << " a = " << req1.a_ << endl;
            ret = TCPSend(bb1_sk, reinterpret_cast<char*>(&req1), sizeof(ReqBB1));
            if (ret < 0) {
                cout << "Send error" << endl;
                bb1_sk->Close();
                delete bb1_sk;
                srv()->InsertServerReuseSocket(pool_index(), sk);
                continue;
            }
            // 2， 发送请求给BB2，以得到b2
            std::string bb2_ip = "127.0.0.1";
            uint16_t bb2_port = 30022;
            Socket *bb2_sk = srv()->GetClientSocket(bb2_ip, bb2_port);
            if (!bb2_sk) {
                bb2_sk = srv()->MakeConnection(bb2_ip, bb2_port);
                if (!bb2_sk) {
                    cout << "Cant get connection: <" << bb2_ip << ", " << bb2_port << ">" << endl;
                    srv()->InsertServerReuseSocket(pool_index(), sk);
                    continue;
                }
            }

            ReqBB2 req2;
            req2.l_ = htonl(sizeof(ReqBB1));
            req2.b_ = htonl(req->b_);
            req2.seq_ = htonl(req->seq_);
            cout << "Send ReqBB2: seq = " << req2.seq_ << " b = " << req2.b_ << endl;
            ret = TCPSend(bb2_sk, reinterpret_cast<char*>(&req2), sizeof(ReqBB2));
            if (ret < 0) {
                cout << "Send error" << endl;
                bb2_sk->Close();
                delete bb2_sk;
                srv()->InsertServerReuseSocket(pool_index(), sk);
                continue;
            }
            // 3， 等待接收
            vector<Socket*> sk_list;
            sk_list.push_back(bb1_sk);
            sk_list.push_back(bb2_sk);

            uint32_t num = 2;
            ret = TCPWaitToRead(sk_list, num, 1000);
            if (ret <= -3) {
                // 出现错误，删除套接口
                sk->Close();
                delete sk;
                bb1_sk->Close();
                delete bb1_sk;
                bb2_sk->Close();
                delete bb2_sk;

                continue;
            }
            // 4，接收数据
            ret = TCPRecv(bb1_sk, buf);
            if (ret < 0) {
                cout << "Recv from BB1 error" << endl;
                bb1_sk->Close();
                delete bb1_sk;
                bb2_sk->Close();
                delete bb2_sk;
                continue;
            }
            RspBB1 *rsp1 = reinterpret_cast<RspBB1*>(&buf[0]);
            ret = TCPRecv(bb2_sk, buf);
            if (ret < 0) {
                cout << "Recv from BB2 error" << endl;
                bb1_sk->Close();
                delete bb1_sk;
                bb2_sk->Close();
                delete bb2_sk;
                continue;
            }
            RspBB2 *rsp2 = reinterpret_cast<RspBB2*>(&buf[0]);
            // 第四步
            // 返回结果
            srv()->InsertClientSocket(bb1_sk);
            srv()->InsertClientSocket(bb2_sk);

            RspBF rsp;
            rsp.l_ = htonl(sizeof(RspBF));
            rsp.seq_ = htonl(req->seq_);
            rsp.sum_ = htonl(ntohl(rsp1->a2_) + ntohl(rsp2->b2_));

            ret = TCPSend(sk, reinterpret_cast<char*>(&rsp), sizeof(RspBB2));
            if (ret < 0) {
                cout << "Send error" << endl;
                sk->Close();
                delete sk;
                continue;
            }
        } // while

        LEAVING;
        return 0;
    };
};

class TestBFServer : public Server {
public:
    TestBFServer(uint32_t epoll_size, uint32_t max_server_socket_num,
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

    Server *srv = new TestBFServer(1024, 1024, 100, 10000);
    string myip = "127.0.0.1";
    uint16_t myport = 20021;

    srv->AddListenSocket(myip, myport);

    srv->InitServer();
    // epoll线程启动，即用于检测套接口的线程
    pthread_t epoll_pid;
    if (pthread_create(&epoll_pid, 0, Server::ServerThreadProc, srv) < 0) {
        cout << "Create epoll thread error" << endl;
        return -1;
    }
    // 启动worker线程
    pthread_t worker_pid[4];
    Processor *worker_processor[4];

    for (int i = 0; i < 4; i++) {

        worker_processor[i] = new TestBFProcessor(srv, 0);

        if (pthread_create(&worker_pid[i], 0, worker_processor[i]->ProcessorThreadProc, worker_processor[i]) < 0) {
            cout << "Create worker thread No." << i << " error" << endl;
            return -1;
        }
    }

    // 等待线程完成
    pthread_join(epoll_pid, 0);
    cout << "epoll thread done" << endl;
    for (int i = 0; i < 4; i++) {
        pthread_join(worker_pid[i], 0);
        cout << "worker thread No." << i << " done" << endl;
    }
    return 0;
}

