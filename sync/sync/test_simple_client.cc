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

#include "client.hpp"

using namespace std;
using namespace NF;

#pragma pack(1)
class Req{
public:
    int l_;
    int seq_;
    int num_;
};
class Rsp{
public:
    int l_;
    int seq_;
    int num2_;
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
        LEAVING;
        return 0;
    };
private:
};


int main (int argc, char **argv) {

    Client *client1 = new TestSimpleClient;
    static int seq = 0;

    string srvip1 = "127.0.0.1";
    uint16_t srvport1 = 20031;

    while (true) {
        getchar();

        Req req1;
        req1.l_ = htonl(sizeof(Req));
        req1.seq_ = htonl(seq++);
        req1.num_ = htonl(seq++);

        Socket *sk = client1->GetClientSocket(srvip1, srvport1);
        if (!sk) {
            sk = client1->MakeConnection(srvip1, srvport1);
            if (!sk) {
                cout << "Can't make connection to server" << endl;
                return -1;
            }
        }

        int ret = client1->TCPSend(sk, reinterpret_cast<char*>(&req1), sizeof(Req));
        if (ret < 0) {
            cout << "Send to server error" << endl;
            return -2;
        }

        cout << "Send Req seq = " << ntohl(req1.seq_) << " num = " << ntohl(req1.num_) << endl;
        vector<char> buf;
        ret = client1->TCPRecv(sk, buf);
        if (ret < 0) {
            cout << "Can't recv from server" << endl;
            return -2;
        }

        Rsp *rsp = reinterpret_cast<Rsp*>(&buf[0]);
        cout << "Recv Rsp seq = " << ntohl(rsp->seq_) << " num2 = " << ntohl(rsp->num2_) << endl;
        client1->InsertClientSocket(sk);
        sk = 0;
    } // while

    return 0;
};
