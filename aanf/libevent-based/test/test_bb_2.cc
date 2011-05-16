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

using std;
using namespace AANF;

class TestBB2 : public Skeleton {
public:
    virtual int ProcessMessage(Message &input_pkt);
};

int main(int argc, char **argv) {

    char *opt_str = "f:m:h";
    string config_file, exe_mode;

    char c;
    while ((c = getopt(argc, argv, opt_str)) != -1) {
        switch (c) {
        case 'f':
            config_file = optarg;
            break;
        case 'm':
            exe_mode = optarg;
            break;
        case 'h':
        default:
            cerr << "Usage: ./test_bf -f configfile -m [debug|daemon] -h" << endl;
            break;
        }
    }
    if (config_file.empty || exe_mode.empty()) {
        cerr << "Parameter error:" << endl;
        cerr << "Usage: ./test_bf -f configfile -m [debug|daemon] -h" << endl;
        return -1;
    }

    TestBB2 *server = new TestBB2;
    int ret = server->LoadConfig(true, config_file);
    if (ret < 0) {
        cerr << "Load config file error!" << endl;
        return -1;
    }

    server->Run();
    return 0;

}
int TestBB2::ProcessMessage(Message &input_pkt) {

    ENTERING;
    PacketFormat pkt;
    pkt.ParseFromArray(input_pkt.data_->start_, input_pkt.data_->used_);

    if (pkt.type() != 100005) {
        SLOG(LogLevel.L_LOGICERR, "Message type error: %d\n", pkt.type());
        LEAVING;
        return 0;
    }
    // 从BF过来的请求
    SLOG(LogLevel.L_INFO, "input Message is type=%d\n", pkt.type());
    BFToBB1Req inner_pkt;
    inner_pkt = pkt.GetExtension(bf_to_bb2_req);

    // 构造发给BF的报文
    PacketFormat rsp;
    BFToBB2Rsp inner_bf_to_bb2_rsp;
    inner_bf_to_bb2_rsp.set_error(BFToBB2Rsp::ErrorCode.OK);
    UserProfile *tmp = inner_bf_to_bb2_rsp.add_user_profile();
    tmp->set_user_id(1);
    tmp->set_user_desc("user No.1");
    tmp->set_gender(1);
    tmp->set_user_name("user name No.1");

    tmp = inner_bf_to_bb2_rsp.add_user_id();
    tmp->set_user_id(2);
    tmp->set_user_desc("user No.2");
    tmp->set_gender(2);
    tmp->set_user_name("user name No.2");

    tmp = inner_bf_to_bb2_rsp.add_user_id();
    tmp->set_user_id(3);
    tmp->set_user_desc("user No.3");
    tmp->set_gender(3);
    tmp->set_user_name("user name No.3");

    rsp.SetExtension(rsp, inner_bf_to_bb2_rsp);
    rsp.set_seq(pkt.seq());
    rsp.set_service_id(1001);
    rsp.set_type(10006);
    rsp.set_version(1001);
    rsp.set_length(rsp.ByteSize());

    MemBlock *to_send = 0;
    int ret = MemPool::GetMemPool()->GetMemBlock(rsp.length(), to_send);
    rsp.SerializeToArray(to_send->start_, to_send->used_);

    string to_ip = "127.0.0.1";
    uint16_t to_port = 20001;

    client->AsyncSend(to_ip, to_port, "127.0.0.1", 0,
                    to_send, Socket::SocketType.T_TCP_SERVER,
                    Socket::DataFormat.DF_BIN, 0);
    SLOG(LogLevel.L_INFO, "after AsyncSend() to BF\n");
    LEAVING;
    return 0;
}
