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

#include "client.hpp"

#pragma pack(1)
class ReqFormat1 {
public:
    int l_;
    int seq_;
    int num_;
};
class ReqFormat2 {
public:
    short l_;
    int seq_;
    int num_;
};
class RspFormat {
public:
    int l_;
    int num2_;
};
#pragma pack(0)

class TestSimpleClient1 : public Client {
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
        if ( len <= 0 || len > max_tcp_pkt_size_) {
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
private:
};

int main (int argc, char **argv) {

    TestClient client;
    ReqBF req;
    RsqBF *rsp;
    req.l_ = htonl(sizeof(ReqBF));
    req.a_ = htonl(10);
    req.b_ = htonl(100);
    req.seq_ = htonl(1000);
    Socket *sk = client.GetClientSocket("127.0.0.1", 20021);
    if (!sk) {
        sk = client.MakeConnect("127.0.0.1", 20021);
        if (!sk) {
            cout << "Can't make connection to BF" << endl;
            return -1;
        }
    }

    int ret = client.TCPSend(sk, &req, sizeof(ReqBF));
    if (ret < 0) {
        cout << "Send to BF error" << endl;
        return -2;
    }

    cout << "Send ReqBF a = " << ntohl(req.a_) << " seq = " << ntohl(req.seq_) << endl;
    vector<char> buf;
    ret = client.TCPRecv(sk, buf);
    if (ret < 0) {
        cout << "Can't recv from BF" << endl;
        return -2;
    }

    rsp = reinterpret_cast<RspBF*>(&buf[0]);
    cout << "Send ReqBF sum = " << ntohl(rsp->sum_) << " seq = " << ntohl(rsp->seq_) << endl;
    return 0;
};
