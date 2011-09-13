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


class TestBFServer : public Server {
public:
    TestBFServer(uint32_t epoll_size, uint32_t max_server_socket_num,
           int timer_interval)
           :Server(epoll_size, max_server_socket_num, timer_interval) {
    };
    virtual int TimerHandler() {

        ENTERING;
        cout << "Timer triggered " << __PRETTY_FUNCTION__ << endl;
        LEAVING;
        return Server::TimerHandler();
    };

    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) {

        ENTERING;
        int len_field = 0;
        int byte_num = 0;
        int ret = 0;

        // 收长度域，4字节
        char *mark = reinterpret_cast<char*>(&len_field);
        while (byte_num < 4) {
            SLOG(4, "Begin to recv len field\n");
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
        byte_num = 0;
        buf_to_fill.resize(len);
		mark = &buf_to_fill[0];
        SLOG(4, "Begin to recv %d bytes date field\n", len);
        while (byte_num < len) {
            ret = recv(sk->sk(), mark, len - byte_num, 0);
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
				mark += ret;
                SLOG(4, "%d bytes have been recved\n", byte_num);
                continue;
            }
        } // while
        SLOG(4, "%d bytes recved\n", byte_num);
        LEAVING;
        return 0;
    };


};
class TestAsClient: public Client 
{
public:
    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) {

        ENTERING;
        int len_field = 0;
        int byte_num = 0;
        int ret = 0;

        // 收长度域，4字节
        char *mark = reinterpret_cast<char*>(&len_field);
        while (byte_num < 4) {
            SLOG(4, "Begin to recv len field\n");
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
        byte_num = 0;
        buf_to_fill.resize(len);
		mark = &buf_to_fill[0];
        SLOG(4, "Begin to recv %d bytes date field\n", len);
        while (byte_num < len) {
            ret = recv(sk->sk(), mark, len - byte_num, 0);
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
				mark += ret;
                SLOG(4, "%d bytes have been recved\n", byte_num);
                continue;
            }
        } // while
        SLOG(4, "%d bytes recved\n", byte_num);
        LEAVING;
        return 0;
    };


};
// 一个Processor对应一个线程
class TestBFProcessor : public Processor {

public:
    TestBFProcessor(Server *srv, Client *client, uint32_t pool_index)
        : Processor(srv, client, pool_index) {
    };
    // 处理逻辑
    virtual int Process() {

        ENTERING;
        while (true) {
            // 第一步
            // 阻塞获取有数据到来的套接口
            SLOG(4, "To get a ready server socket\n");
            Socket *sk = srv()->GetServerReadySocket(pool_index());
            if (!sk) {
                SLOG(4, "GetServerReadySocket() error(cann't be here)\n");
                continue;
            }
            // 第二步
            // 阻塞接收数据
            std::vector<char> buf;
            SLOG(4, "To recv data from client\n");
            int ret = srv()->TCPRecv(sk, buf);
            if (ret < 0) {
                // 错误，不可恢复
                SLOG(4, "TCPRecv() error\n");
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
			int recv_seq = req->seq_;

            SLOG(4, "Recved ReqBF: seq = %d a = %d b = %d\n", req->seq_, req->a_, req->b_);

            // 1，发送请求给BB1，以得到a2
            std::string bb1_ip = "127.0.0.1";
            uint16_t bb1_port = 30021;
            SLOG(4, "To get an idle client socket to BB1 <%s : %u>\n", bb1_ip.c_str(), bb1_port);

            Socket *bb1_sk = client()->GetClientSocket(bb1_ip, bb1_port);
            if (!bb1_sk) {
                SLOG(4, "No idle client socket, make one\n");
                bb1_sk = client()->MakeConnection(bb1_ip, bb1_port);
                if (!bb1_sk) {
                    SLOG(4, "MakeConnection() error\n");
                    srv()->InsertServerReuseSocket(pool_index(), sk);
                    continue;
                }
            }

            ReqBB1 req1;
            req1.l_ = htonl(sizeof(ReqBB1));
            req1.a_ = htonl(req->a_);
            req1.seq_ = htonl(req->seq_);

            SLOG(4, "To send ReqBB1: seq = %d a = %d\n", ntohl(req1.seq_), ntohl(req1.a_));
			char buf_to_send[1024*1024];
			int len_to_send = sizeof(ReqBB1);
			len_to_send = htonl(len_to_send);
			memcpy(buf_to_send, reinterpret_cast<char*>(&len_to_send), 4);
			memcpy(buf_to_send+4, reinterpret_cast<char*>(&req1), sizeof(ReqBB1));
            ret = client()->TCPSend(bb1_sk, buf_to_send, 4+sizeof(ReqBB1));

            if (ret < 0) {
                SLOG(4, "TCPSend() error\n");
                bb1_sk->Close();
                delete bb1_sk;
                srv()->InsertServerReuseSocket(pool_index(), sk);
                continue;
            }
            SLOG(4, "Send ReqBB1 OK\n");

            // 2， 发送请求给BB2，以得到b2
            std::string bb2_ip = "127.0.0.1";
            uint16_t bb2_port = 30022;
            SLOG(4, "To get an idle client socket to BB2 <%s : %u>\n", bb2_ip.c_str(), bb2_port);

            Socket *bb2_sk = client()->GetClientSocket(bb2_ip, bb2_port);
            if (!bb2_sk) {
                SLOG(4, "No idle client socket, make one\n");
                bb2_sk = client()->MakeConnection(bb2_ip, bb2_port);
                if (!bb2_sk) {
                    SLOG(4, "MakeConnection() error\n");
                    srv()->InsertServerReuseSocket(pool_index(), sk);
                    continue;
                }
            }

            ReqBB2 req2;
            req2.l_ = htonl(sizeof(ReqBB1));
            req2.b_ = htonl(req->b_);
            req2.seq_ = htonl(req->seq_);
            SLOG(4, "To send ReqBB2: seq = %d b = %d\n", ntohl(req2.seq_), ntohl(req2.b_));
			memset(buf_to_send, 0, sizeof(buf_to_send));
			len_to_send = sizeof(ReqBB2);
			len_to_send = htonl(len_to_send);
			memcpy(buf_to_send, reinterpret_cast<char*>(&len_to_send), 4);
			memcpy(buf_to_send+4, reinterpret_cast<char*>(&req2), sizeof(ReqBB2));
            ret = client()->TCPSend(bb2_sk, buf_to_send, 4+sizeof(ReqBB2));
            if (ret < 0) {
                SLOG(4, "TCPSend() error\n");
                bb2_sk->Close();
                delete bb2_sk;
                srv()->InsertServerReuseSocket(pool_index(), sk);
                continue;
            }
            // 3， 等待接收

            vector<Socket*> sk_list;
            vector<uint32_t> sk_list_error, sk_list_triggered;
            sk_list.push_back(bb1_sk);
            sk_list.push_back(bb2_sk);
            SLOG(4, "To wait RspBB1 and RspBB2\n");

            uint32_t num = 2;
            ret = client()->TCPWaitToRead(sk_list, sk_list_triggered, sk_list_error, num, 1000);
            if (ret < 0) {
                // 出现错误，删除套接口
                SLOG(4, "TCPWaitToRead(() error: %d\n", ret);
                //sk->Close();
                //delete sk;
                bb1_sk->Close();
                delete bb1_sk;
                bb2_sk->Close();
                delete bb2_sk;
                srv()->InsertServerReuseSocket(pool_index(), sk);
                continue;
            }

            // 4，接收数据
            SLOG(4, "Recv RspBB1\n");
            ret = client()->TCPRecv(bb1_sk, buf);
            if (ret < 0) {
                SLOG(4, "TCPRecv() error\n");
                bb1_sk->Close();
                delete bb1_sk;
                bb2_sk->Close();
                delete bb2_sk;
                srv()->InsertServerReuseSocket(pool_index(), sk);
                continue;
            }
            RspBB1 *rsp1 = reinterpret_cast<RspBB1*>(&buf[0]);
			int recv_a = ntohl(rsp1->a2_);
            SLOG(4, "Recv RspBB1->a2_: %d\n", recv_a);


			buf.clear();
            SLOG(4, "Recv RspBB2\n");
            ret = client()->TCPRecv(bb2_sk, buf);
            if (ret < 0) {
                SLOG(4, "TCPRecv() error\n");
                bb1_sk->Close();
                delete bb1_sk;
                bb2_sk->Close();
                delete bb2_sk;
                srv()->InsertServerReuseSocket(pool_index(), sk);
                continue;
            }
            RspBB2 *rsp2 = reinterpret_cast<RspBB2*>(&buf[0]);
            int recv_b = ntohl(rsp2->b2_);
            //int recv_b = rsp2->b2_;
            SLOG(4, "Recv RspBB2->b2_: %d\n", recv_b);
            // 第四步
            // 返回结果
            client()->InsertClientSocket(bb1_sk);
            client()->InsertClientSocket(bb2_sk);

            RspBF rsp;
            rsp.l_ = htonl(sizeof(RspBF));
            rsp.seq_ = htonl(recv_seq);
            //rsp.sum_ = htonl(ntohl(rsp1->a2_) + ntohl(rsp2->b2_));
            rsp.sum_ = htonl(recv_a + recv_b);

            SLOG(4, "To send RspBF seq = %d sum = %d\n", ntohl(rsp.seq_), ntohl(rsp.sum_));
			len_to_send = sizeof(RspBF);
			len_to_send = htonl(len_to_send);
			memset(buf_to_send, 0, sizeof(buf_to_send));
			memcpy(buf_to_send, reinterpret_cast<char*>(&len_to_send), 4);
			memcpy(buf_to_send+4, reinterpret_cast<char*>(&rsp), sizeof(RspBF));
            ret = srv()->TCPSend(sk, buf_to_send, 4+sizeof(RspBF));
            if (ret < 0) {
                SLOG(4, "TCPSend() error\n");
                sk->Close();
                delete sk;
                continue;
            }
			srv()->InsertServerReuseSocket(pool_index(), sk);
        } // while

        LEAVING;
        return 0;
    };
};
int main(int argc, char **argv) {

    Server *srv = new TestBFServer(1024, 1024, 10000);
	Client *client = new TestAsClient();
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

    for (int i = 0; i < 1; i++) {

        worker_processor[i] = new TestBFProcessor(srv, client, 0);

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

