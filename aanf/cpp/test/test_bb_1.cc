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

class TestBB1 : public Skeleton {
public:
    virtual int ProcessPacket(Packet &input_pkt);
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
            cerr << "Usage: ./test_bb1 -f configfile -h" << endl;
            break;
        }
    }

    if (config_file.empty || exe_mode.empty()) {
        cerr << "Parameter error:" << endl;
        cerr << "Usage: ./test_bb1 -f configfile -h" << endl;
        return -1;
    }

    TestBB1 *server = new TestBB1;
    int ret = server->LoadConfig(true, config_file);
    if (ret < 0) {
        cerr << "Load config file error!" << endl;
        return -1;
    }

    server->Run();
    return 0;

}

int TestBB1::ProcessPacket(Packet &input_pkt) {

    ENTERING;
    PacketFormat pkt;
    pkt.ParseFromArray(input_pkt.data_->start_, input_pkt.data_->used_);

    if (pkt.type() != 10003) {
        SLOG(LogLevel.L_LOGICERR, "packet type error: %d\n", pkt.type());
        LEAVING;
        return 0;
    }
    // 从BF过来的请求
    SLOG(LogLevel.L_INFO, "input packet is type=%d\n", pkt.type());
    BFToBB1Req inner_pkt;
    inner_pkt = pkt.GetExtension(bf_to_bb1_req);

    // 构造发给BF的报文
    PacketFormat rsp;
    BFToBB1Rsp inner_bf_to_bb1_rsp;
    SLOG(LogLevel.L_INFO, "recved a query for: %s\n", inner_pkt.user_name_keyword())
    inner_bf_to_bb1_rsp.set_error(BFToBB1Rsp::ErrorCode.OK);
    uint64_t *tmp = inner_bf_to_bb1_rsp.add_user_id();
    *tmp = 1;
    tmp = inner_bf_to_bb1_rsp.add_user_id();
    *tmp = 2;
    tmp = inner_bf_to_bb1_rsp.add_user_id();
    *tmp = 3;
    rsp.SetExtension(rsp, inner_bf_to_bb1_rsp);
    rsp.set_seq(pkt.seq());
    rsp.set_service_id(1001);
    rsp.set_type(10004);
    rsp.set_version(1001);
    rsp.set_length(rsp.ByteSize());

    MemBlock *to_send = 0;
    int ret = MemPool::GetMemPool()->GetMemBlock(rsp.length(), to_send);
    rsp.SerializeToArray(to_send->start_, to_send->used_);

    string to_ip = "127.0.0.1";
    uint16_t to_port = 30001;

    client->AsyncSend(to_ip, to_port, "127.0.0.1", 0,
                    to_send, Socket::SocketType.T_TCP_SERVER,
                    Socket::DataFormat.DF_BIN, 0);
    SLOG(LogLevel.L_INFO, "after AsyncSend() to BF\n");


    LEAVING;
    return 0;
}
