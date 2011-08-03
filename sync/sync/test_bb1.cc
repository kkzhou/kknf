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
#pragma pack(0)

// һ��Processor��Ӧһ���߳�
class TestBB1Processor : public Processor {

public:
    TestBB1Processor(Server *srv, uint32_t pool_index)
        : Processor(srv, pool_index) {
    };
    //�������ݣ���Ҫ����ҵ�����Packetize����������
    // �����Գ�������LV��ʽ
    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) {

        ENTERING;
        int len_field = 0;
        int byte_num = 0;
        int ret = 0;

        // �ճ�����4�ֽ�
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

        // ��������
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

    // �����߼�
    virtual int Process() {

        ENTERING;
        while (true) {
            // ��һ��
            // ������ȡ�����ݵ������׽ӿ�
            Socket *sk = srv()->GetServerReadySocket(pool_index());
            if (!sk) {
                continue;
            }
            // �ڶ���
            // ������������
            std::vector<char> buf;
            int ret = TCPRecv(sk, buf);
            if (ret < 0) {
                // ���󣬲��ɻָ�
                sk->Close();
                delete sk;
                continue;
            }
            // ������
            // �����߼�
            ReqBB1 *req = reinterpret_cast<ReqBB1*>(&buf[0]);
            req->l_ = ntohl(req->l_);
            req->a_ = ntohl(req->a_);
            req->seq_ = ntohl(req->seq_);
            cout << "Recv seq = " << req->seq_ << " a = " << req->a_ << endl;
            RspBB1 rsp;
            rsp.a2_ = req->a_ + 1;
            rsp.l_ = sizeof(RspBB1);
            rsp.l_ = htonl(rsp.l_);
            rsp.a2_ = htonl(rsp.a2_);
            rsp.seq_ = htonl(req->seq_);
            cout << "Send seq = " << rsp.seq_ << " a2 = " << rsp.a2_ << endl;
            // ���Ĳ�
            // ���ؽ��
            ret = TCPSend(sk, reinterpret_cast<char*>(&rsp), sizeof(RspBB1));
            if (ret < 0) {
                cout << "Send error" << endl;
            }
        } // while

        LEAVING;
        return 0;
    };
};

class TestBB1Server : public Server {
public:
    TestBB1Server(uint32_t epoll_size, uint32_t max_server_socket_num,
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

    Server *srv = new TestBB1Server(1024, 1024, 100, 10000);
    string myip = "127.0.0.1";
    uint16_t myport = 20021;
    srv->AddListenSocket(myip, myport);

    srv->InitServer();
    // epoll�߳������������ڼ���׽ӿڵ��߳�
    pthread_t epoll_pid;
    if (pthread_create(&epoll_pid, 0, Server::ServerThreadProc, srv) < 0) {
        cout << "Create epoll thread error" << endl;
        return -1;
    }
    // ����worker�߳�
    pthread_t worker_pid[4];
    TestBB1Processor *worker_processor[4];

    for (int i = 0; i < 4; i++) {

        worker_processor[i] = new TestBB1Processor(srv, 0);

        if (pthread_create(&worker_pid[i], 0, worker_processor[i]->ProcessorThreadProc, worker_processor[i]) < 0) {
            cout << "Create worker thread No." << i << " error" << endl;
            return -1;
        }
    }

    // �ȴ��߳����
    pthread_join(epoll_pid, 0);
    cout << "epoll thread done" << endl;
    for (int i = 0; i < 4; i++) {
        pthread_join(worker_pid[i], 0);
        cout << "worker thread No." << i << " done" << endl;
    }
    return 0;
}

