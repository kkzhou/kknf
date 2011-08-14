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

class TestClient : public Client {
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
            SLOG(4, "Begin to recv len field\n");
            ret = recv(sk->sk(), &len_field, 4 - byte_num, 0);
            if (ret < 0) {
                if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK) {
                    SLOG(4, "recv() len field error: %s\n", strerror(errno));
                    LEAVING;
                    return -1;
                }
            } else {
                byte_num += ret;
                SLOG(4, "Recved %d bytes\n", byte_num);
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

int main (int argc, char **argv) {

    TestClient client;
    ReqBF req;
    RspBF *rsp;
    req.l_ = htonl(sizeof(ReqBF));
    req.a_ = htonl(10);
    req.b_ = htonl(100);
    req.seq_ = htonl(1000);
    string bfip = "127.0.0.1";
    uint16_t bfport = 20021;
    SLOG(4, "To get idle client socket to BF <%s : %u>\n", bfip.c_str(), bfport);
    Socket *sk = client.GetClientSocket(bfip, bfport);
    if (!sk) {
        SLOG(4, "No idle client socket, make one\n");
        sk = client.MakeConnection(bfip, bfport);
        if (!sk) {
            SLOG(4, "MakeConnection() error\n");
            return -1;
        }
    }

    SLOG(4, "To send ReqBF: seq = %d a = %d b = %d\n", req.seq_, req.a_, req.b_);
    int ret = client.TCPSend(sk, reinterpret_cast<char*>(&req), sizeof(ReqBF));
    if (ret < 0) {
        SLOG(4, "TCPSend() error\n");
        return -2;
    }
    SLOG(4, "Send ReqBF OK\n");

    vector<char> buf;
    SLOG(4, "To recv RspBF\n");
    ret = client.TCPRecv(sk, buf);
    if (ret < 0) {
        SLOG(4, "TCPRecv() error\n");
        return -2;
    }

    rsp = reinterpret_cast<RspBF*>(&buf[0]);
    SLOG(4, "Recved RspBF: seq = %d sum = %d\n", rsp->seq_, rsp->sum_);
    return 0;
};
