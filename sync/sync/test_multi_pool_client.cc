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
private:
};


class TestSimpleClient2 : public Client {
public:
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
private:
};

class TestSimpleClient3 : public Client {
public:
private:
};

int main (int argc, char **argv) {

    Client *client1 = new TestSimpleClient1;
    Client *client2 = new TestSimpleClient2;
    Client *client3 = new TestSimpleClient3;
    static int seq = 0;

    string srvip1 = "127.0.0.1";
    uint16_t srvport1 = 20031;
    string srvip2 = "127.0.0.1";
    uint16_t srvport2 = 20032;
    string srvip3 = "127.0.0.1";
    uint16_t srvport3 = 20033;
    while (true) {

        // 第一种数据格式
        ReqFormat1 req1;
        req1.l_ = htonl(sizeof(ReqFormat1));
        req1.seq_ = htonl(seq++);
        req1.num_ = htonl(-1 * seq++);

        Socket *sk = client1->GetClientSocket(srvip1, srvport1);
        if (!sk) {
            sk = client1->MakeConnection(srvip1, srvport1);
            if (!sk) {
                cout << "Can't make connection to server" << endl;
                return -1;
            }
        }

        int ret = client1->TCPSend(sk, reinterpret_cast<char*>(&req1), sizeof(ReqFormat1));
        if (ret < 0) {
            cout << "Send to server error" << endl;
            return -2;
        }

        cout << "Send ReqFormat1 seq = " << ntohl(req1.seq_) << " num = " << ntohl(req1.num_) << endl;
        vector<char> buf;
        ret = client1->TCPRecv(sk, buf);
        if (ret < 0) {
            cout << "Can't recv from server" << endl;
            return -2;
        }

        RspFormat *rsp = reinterpret_cast<RspFormat*>(&buf[0]);
        cout << "Send RspFormat seq = " << ntohl(rsp->seq_) << " num = " << ntohl(rsp->num2_) << endl;
        client1->InsertClientSocket(sk);
        sk = 0;

        // 第二种格式的数据包
        string cmd_line = "Hello World!\n";
        //ReqFormat2 req2;
        sk = client2->GetClientSocket(srvip2, srvport2);
        if (!sk) {
            sk = client2->MakeConnection(srvip2, srvport2);
            if (!sk) {
                cout << "Can't make connection to server" << endl;
                return -1;
            }
        }

        ret = client2->TCPSend(sk, const_cast<char*>(cmd_line.data()), cmd_line.size());
        if (ret < 0) {
            cout << "Send to server error" << endl;
            return -2;
        }

        cout << "Send ReqFormat2 cmd_line = " << cmd_line << endl;
        ret = client2->TCPRecv(sk, buf);
        if (ret < 0) {
            cout << "Can't recv from server" << endl;
            return -2;
        }

        char *rsp_line = &buf[0];
        int i = 0;
        for (i = 0; i < ret; i++) {
            if (rsp_line[i] == '\n') {
                rsp_line[i] = '\0';
            }
        }
        rsp_line[ret - 1] = '\0';

        i = 0;
        while (i < ret) {
            string cmd_line;
            cmd_line.append(&rsp_line[i]);
            i += cmd_line.size() + 1;

            cout << "Recv Rsp = " << cmd_line << endl;
        }
        client2->InsertClientSocket(sk);
        sk = 0;

        // 第三种格式，UDP
        ReqFormat3 req3;
        req3.seq_ = htonl(seq++);
        req3.num_ = htonl(-1 * seq++);
        int udp_sk = client3->MakeUDPSocket();

        if (client3->UDPSend(udp_sk, srvip3, srvport3, reinterpret_cast<char*>(&req3), sizeof(ReqFormat3)) < 0) {
            close(udp_sk);
            cout << "UDP send error" << endl;
            return -2;
        }

        string from_ip;
        uint16_t from_port;
        if (client3->UDPRecv(udp_sk, buf, from_ip, from_port) < 0) {
            close(udp_sk);
            cout << "UDP recv error" << endl;
            return -2;
        }
        rsp = reinterpret_cast<RspFormat*>(&buf[0]);
        cout << "Send RspFormat seq = " << ntohl(rsp->seq_) << " num2 = " << ntohl(rsp->num2_) << endl;
    } // while

    return 0;
};
