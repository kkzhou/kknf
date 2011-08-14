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
#include <iostream>
#include <stdlib.h>

#include "client.hpp"

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

class TestSimpleClient : public Client {
public:
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
        SLOG(4, "The len is %d bytes\n", len);
        if ( len <= 0 || len > max_tcp_pkt_size()) {
            SLOG(4, "len field error: %d\n", len);
            LEAVING;
            return -2;
        }

        // 收数据域
        byte_num = 4;
        buf_to_fill.resize(len);
        SLOG(4, "Begin to recv %d bytes date field\n", len);
        while (byte_num < len) {
            ret = recv(sk->sk(), &buf_to_fill[0], len - byte_num, 0);
            if (ret < 0) {
                SLOG(4, "Recv returns %d, error: %s\n", ret, strerror(errno));
                if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK) {
                    SLOG(4, "recv() data field error: %s\n", strerror(errno));
                    LEAVING;
                    return -2;
                }
            } else if (ret == 0) {
                SLOG(4, "Closed by peer\n");
                LEAVING;
                return -2;
            } else {
                byte_num += ret;
                SLOG(4, "%d bytes have been recved\n", byte_num);
                continue;
            }
        } // while
        SLOG(4, "%d bytes recved\n", byte_num);
        LEAVING;
        return 0;
    };
private:
};

void* ClientThreadProc(void *arg) {

    Client *client1 = reinterpret_cast<Client*>(arg);

    static int seq = 0;

    string srvip[3];
    uint16_t srvport[3];
    uint16_t srvnum = 3;

    for (uint16_t i = 0; i < srvnum ; i++) {
        srvip[i] = "127.0.0.1";
        srvport[i] = 20031 + i;
    }

    while (true) {
        usleep(1000);

        Req req1;
        req1.l_ = htonl(sizeof(Req));
        req1.seq_ = htonl(seq++);
        req1.num_ = htonl(seq++);
        int index = rand();
        index %= srvnum;

        Socket *sk = client1->GetClientSocket(srvip[index], srvport[index]);
        if (!sk) {
            sk = client1->MakeConnection(srvip[index], srvport[index]);
            if (!sk) {
                SLOG(4, "MakeConnection() error\n");
                continue;
            }
        }

        int ret = client1->TCPSend(sk, reinterpret_cast<char*>(&req1), sizeof(Req));
        if (ret < 0) {
            SLOG(4, "TCPSend() error, to delete this socket\n");
            sk->Close();
            delete sk;
            continue;

        }

        SLOG(4, "Send Req seq = %d num = %d\n", ntohl(req1.seq_), ntohl(req1.num_));
        vector<char> buf;
        ret = client1->TCPRecv(sk, buf);
        if (ret < 0) {
            SLOG(4, "TCPRecv() error, to delete this socket\n");
            sk->Close();
            delete sk;
            continue;
        }

        Rsp *rsp = reinterpret_cast<Rsp*>(&buf[0]);
        SLOG(4, "Recv Rsp seq = %d num2 = %d\n", ntohl(rsp->seq_), ntohl(rsp->num2_));
        client1->InsertClientSocket(sk);
        sk = 0;
    } // while
    return 0;
};

int main (int argc, char **argv) {

    Client *client = new TestSimpleClient;

    pthread_t worker_pid[100];
    int num = 100;

    for (int i = 0; i < num; i++) {

        if (pthread_create(&worker_pid[i], 0, ClientThreadProc, client) < 0) {
            SLOG(4, "Create thread error\n");
            return -1;
        }
        SLOG(4, "No.%d thread started, pid = %lu\n", i, worker_pid[i]);
    }

    // 等待线程完成
    for (int i = 0; i < num; i++) {
        pthread_join(worker_pid[i], 0);
        SLOG(4, " No.%d thread exited, pid = %lu\n", i, worker_pid[i]);
    }

    return 0;
};
