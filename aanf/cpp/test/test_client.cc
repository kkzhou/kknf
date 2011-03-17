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

#include <string>
#include <iostream>
#include "skeleton.h"
#include "ProtoForTest.pb.h"

using std;
using namespace AANF;
using namespace Protocol;

class TestClient : public Skeleton {
public:
    class TestClientThreadArg {
    public:
        TestClient *client_;
    };
public:
    // 处理回应报文的，因为是Client，是请求的发起者，接收的是应答
    virtual int ProcessPacket(Packet &input_pkt);
    static void TestClientThreadProc(TestClientThreadArg *arg);
};

int main(int argc, char **argv) {

    char *opt_str = "f:h";
    string config_file;

    char c;
    while ((c = getopt(argc, argv, opt_str)) != -1) {
        switch (c) {
        case 'f':
            config_file = optarg;
            break;
        case 'h':
        default:
            cerr << "Usage: ./test_client -f configfile -h" << endl;
            break;
        }
    }
    if (config_file.empty) {
        cerr << "Parameter error:" << endl;
        cerr << "Usage: ./test_client -f configfile -h" << endl;
        return -1;
    }

    TestClient *client = new TestClient;
    int ret = client->LoadConfig(true, config_file);
    if (ret < 0) {
        cerr << "Load config file error!" << endl;
        return -1;
    }

    client->Init();
    // 启动一个线程来启动netframe
    pthread_t tid;
    TestClientThreadArg *arg = new TestClientThreadArg;
    arg->client_ = client;
    pthread_create(&tid, NULL, TestClientThreadProc, arg);
    sleep(3);
    // 发送数据包
    PacketFormat req;
    CToBFReq inner_req;
    inner_req.set_query_string("nihao");
    req.SetExtension(c_to_bf_req, inner_req);

    req.set_service_id(1001);
    req.set_type(100001);
    req.set_version(1001);
    req.set_length(req.ByteSize());

    MemBlock *to_send = 0;
    int ret = MemPool::GetMemPool()->GetMemBlock(req.length(), to_send);
    req.SerializeToArray(to_send->start_, to_send->used_);
    string to_ip = "127.0.0.1";
    uint16_t to_port = 10001;

    client->AsyncSend(to_ip, to_port, "127.0.0.1", 0,
                      to_send, Socket::SocketType.T_TCP_CLIENT,
                      Socket::DataFormat.DF_BIN, 0);
    cerr << "After AsyncSend()" << endl;
    client->cancel_ = true;
    pthread_join(tid, 0);
    cerr << "exit!" << endl;
    return 0;

}

int TestClient::ProcessPacket(Packet &input_pkt) {

    ENTERING;
    SLOG(LogLevel.L_INFO, "Response recved!\n");
    PacketFormat rsp;
    rsp.ParseFromArray(input_pkt.data_->start_, input_pkt.data_->used_);

    if (rsp.type() != 10002) {
        SLOG(LogLevel.L_LOGICERR, "Response type error: %d\n", rsp.type());
        LEAVING;
        return 0;
    }

    CToBFRsp inner_rsp;
    inner_rsp = rsp.GetExtension(c_to_bf_rsp);
    LEAVING;
    return 0;
}

void TestClient::TestClientThreadProc(TestClientThreadArg *arg) {

    ENTERING;
    arg->client_->Run();
    LEAVING;
}
