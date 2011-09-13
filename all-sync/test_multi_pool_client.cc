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
		char *mark = reinterpret_cast<char*>(&len_field);
        while (byte_num < 4) {
            SLOG(2, "Begin to recv len field\n");
            ret = recv(sk->sk(), mark, 4 - byte_num, 0);
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
				mark += ret;
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
        byte_num = 0;
        buf_to_fill.resize(len);
		mark = &buf_to_fill[0];
        SLOG(2, "Begin to recv %d bytes date field\n", len);
        while (byte_num < len) {
            ret = recv(sk->sk(), mark, len - byte_num, 0);
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
				mark += ret;
                SLOG(2, "%d bytes have been recved\n", byte_num);
                continue;
            }
        } // while
        SLOG(4, "%d bytes recved\n", byte_num);
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
        SLOG(4, "Begin to recv a line\n");
        buf_to_fill.resize(1024);
        ret = recv(sk->sk(), &buf_to_fill[0], buf_to_fill.size(), 0);
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
private:
};

class TestSimpleClient3 : public Client {
public:
	virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) 
	{return 0;}
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

        getchar();
        SLOG(4, "To get an idle client socket to send ReqFormat1, <%s : %u>\n", srvip1.c_str(), srvport1);
        Socket *sk = client1->GetClientSocket(srvip1, srvport1);
        if (!sk) {
            SLOG(4, "No idle socket found, to make one\n");
            sk = client1->MakeConnection(srvip1, srvport1);
            if (!sk) {
                SLOG(4, "Makeconnection error\n");
                return -1;
            }
        }

        SLOG(4, "To send ReqFormat1 seq = %d num = %d\n", ntohl(req1.seq_), ntohl(req1.num_));
		int len_to_send = sizeof(ReqFormat1);
		len_to_send = htonl(len_to_send);
		char buf_to_send[1024*1024];
		memcpy(buf_to_send, reinterpret_cast<char*>(&len_to_send), 4);
		memcpy(buf_to_send+4, reinterpret_cast<char*>(&req1), sizeof(ReqFormat1));
        int ret = client1->TCPSend(sk, buf_to_send, 4+sizeof(ReqFormat1));
        if (ret < 0) {
            SLOG(4, "Send data error\n");
            return -2;
        }
        SLOG(4, "Send ReqFormat1 OK, then to recv RspFormat\n");
        vector<char> buf;
        ret = client1->TCPRecv(sk, buf);
        if (ret < 0) {
            SLOG(4, "TCPRecv() error\n");
            return -2;
        }

        RspFormat *rsp = reinterpret_cast<RspFormat*>(&buf[0]);
        SLOG(4, "Recved RspFormat seq = %d num2 = %d\n", ntohl(rsp->seq_), ntohl(rsp->num2_));
        SLOG(4, "Return cliengt socket\n");
        client1->InsertClientSocket(sk);
        sk = 0;

        getchar();
        // 第二种格式的数据包
        string cmd_line = "Hello World!\n";
        //ReqFormat2 req2;
        SLOG(4, "To get an idle client socket to send ReqFormat2, <%s : %u>\n", srvip2.c_str(), srvport2);
        sk = client2->GetClientSocket(srvip2, srvport2);
        if (!sk) {
            SLOG(4, "No idle socket found, to make one\n");
            sk = client2->MakeConnection(srvip2, srvport2);
            if (!sk) {
                SLOG(4, "Makeconnection error\n");
                return -1;
            }
        }

        SLOG(4, "To send ReqFormat2 cmdline = %s\n", cmd_line.c_str());
        ret = client2->TCPSend(sk, const_cast<char*>(cmd_line.data()), cmd_line.size());
        if (ret < 0) {
            SLOG(4, "Send data error\n");
            return -2;
        }

        SLOG(4, "Send ReqFormat2 OK, then to recv(not Rspformat)\n");
        ret = client2->TCPRecv(sk, buf);
        if (ret < 0) {
            SLOG(4, "TCPRecv() error\n");
            return -2;
        }

        char *tmp = &buf[0];
        string rspline;
        uint32_t i = 0;
        for (i = 0; i < buf.size(); i++) {
            if (tmp[i] == '\n') {
                tmp[i] = '\0';
                rspline.append(tmp);
                SLOG(4, "Line extracted: %s\n", rspline.c_str());
                break;
            }
        }
        if (i != buf.size()) {
            SLOG(4, "Data corrupted\n");
        } else {
            if (rspline.empty()) {
                SLOG(4, "Not a complete line recved\n");
                return -2;
            }
        }
        client2->InsertClientSocket(sk);
        sk = 0;

        getchar();
        // 第三种格式，UDP
        ReqFormat3 req3;
        req3.seq_ = htonl(seq++);
        req3.num_ = htonl(-1 * seq++);
        int udp_sk = client3->MakeUDPSocket();

        SLOG(4, "To send UDP data ReqFormat3 seq = %d num = %d\n", htonl(req3.seq_), htonl(req3.num_));
        if (client3->UDPSend(udp_sk, srvip3, srvport3, reinterpret_cast<char*>(&req3), sizeof(ReqFormat3)) < 0) {
            close(udp_sk);
            SLOG(4, "Send error\n");
            return -2;
        }

        string from_ip;
        uint16_t from_port;
        SLOG(4, "Then recv UDP data(maybe not the response we expected)\n");
        if (client3->UDPRecv(udp_sk, buf, from_ip, from_port) < 0) {
            close(udp_sk);
            SLOG(4, "UDPRecv() error\n");
            return -2;
        }
        rsp = reinterpret_cast<RspFormat*>(&buf[0]);
        SLOG(4, "Recved RspFormat seq = %d num2 = %d, from <%s : %u>\n",
             ntohl(rsp->seq_), ntohl(rsp->num2_), from_ip.c_str(), from_port);
    } // while

    return 0;
};
