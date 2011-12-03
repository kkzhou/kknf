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
#include <list>
#include "server.h"
#include "ProtoForTest.pb.h"

using std;
using namespace AANF;
using namespace Protocol;

class BFLocalData {
public:
    class UserProfile {
    public:
        uint64_t user_id_;
        string user_name_;
        string user_desc_;
        int gender_;
    };
public:
    list<uint64_t> userid_list_;
    lise<UserProfile> userprofile_list_;
    uint64_t seq_from_client_;
    string client_ipstr_;
    uint16_t client_port_;
    uint64_t my_seq_;
};

class TestBF : public Skeleton {
public:
    virtual int ProcessMessage(Message &input_pkt);
private:
    int ProcessMessageFromClient(Message &input_pkt);
    int ProcessMessageFromBB1(Message &input_pkt);
    int ProcessMessageFromBB2(Message &input_pkt);
private:
    map<uint64_t, BFLocalData*> ld_map_;
    static uint64_t sequence_;
};

uint64_t TestBF::sequence_ = 0;

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

    TestBF *bf = new TestBF;
    int ret = bf->LoadConfig(true, config_file);
    if (ret < 0) {
        cerr << "Load config file error!" << endl;
        return -1;
    }
    bf->Init();
    bf->Run();
    return 0;
}

int TestBF::ProcessMessage(Message &input_pkt) {

    ENTERING;
    PacketFormat pkt;
    pkt.ParseFromArray(input_pkt.data_->start_, input_pkt.data_->used_);

    if (pkt.type() == 100001) {
        // 从C过来的请求
        SLOG(LogLevel.L_INFO, "input Message is type=%d\n", pkt.type());
        CToBFReq inner_pkt;
        inner_pkt = req.GetExtension(c_to_bf_req);

        BFLocalData *ld = new BFLocalData;
        ld->seq_from_client_ = pkt.seq();
        ld->client_ipstr_ = pkt.peer_ipstr_;
        ld->client_port_ = pkt.peer_port_;
        ld->my_seq_ = sequence_++;
        ld_map_.insert(pair<uint64_t, BFLocalData*>(ld->my_seq_, ld));

        // 构造发给BB1的报文
        PacketFormat req;
        BFToBB1Req inner_bf_to_bb1_req;
        inner_bf_to_bb1_req.set_user_name_keyword(inner_pkt.query_string());
        req.SetExtension(bf_to_bb1_req, inner_bf_to_bb1_req);
        req.set_seq(ld->my_seq_);
        req.set_service_id(1002);
        req.set_type(10003);
        req.set_version(1001);
        req.set_length(req.ByteSize());

        MemBlock *to_send = 0;
        int ret = MemPool::GetMemPool()->GetMemBlock(req.length(), to_send);
        req.SerializeToArray(to_send->start_, to_send->used_);

        string to_ip = "127.0.0.1";
        uint16_t to_port = 30001;

        client->AsyncSend(to_ip, to_port, "127.0.0.1", 0,
                        to_send, Socket::SocketType.T_TCP_CLIENT,
                        Socket::DataFormat.DF_BIN, 0);
        SLOG(LogLevel.L_INFO, "after AsyncSend() to BB1\n");

    } else if (req.type() == 10004) {
        // 从BB1回来的报文
        SLOG(LogLevel.L_INFO, "input Message is type=%d\n", req.type());
        BFToBB1Rsp inner_pkt;
        inner_pkt = req.GetExtension(bf_to_bb1_rsp);
        BFLocalData *ld = ld_map_[pkt.seq()];

        for (int i = 0; i < inner_pkt.user_id_size(); i++) {

            ld->userid_list_.push_back(inner_rsp.user_id(i));
        }

        // 构造发给BB2的报文
        PacketFormat req;
        BFToBB2Req inner_bf_to_bb2_req;

        list<uint64_t>::iterator it, endit = ld->userid_list_.end();


        for (it = ld->userid_list_.begin(); it != endit; it++) {
            uint64_t *userid = inner_bf_to_bb2_req.add_user_id();
            *userid = *it;
        }

        req.SetExtension(bf_to_bb2_req, inner_bf_to_bb2_req);
        req.set_seq(ld->my_seq_);
        req.set_service_id(1003);
        req.set_type(10005);
        req.set_version(1001);
        req.set_length(req.ByteSize());

        MemBlock *to_send = 0;
        int ret = MemPool::GetMemPool()->GetMemBlock(req.length(), to_send);
        req.SerializeToArray(to_send->start_, to_send->used_);

        string to_ip = "127.0.0.1";
        uint16_t to_port = 30002;

        client->AsyncSend(to_ip, to_port, "127.0.0.1", 0,
                        to_send, Socket::SocketType.T_TCP_CLIENT,
                        Socket::DataFormat.DF_BIN, 0);
        SLOG(LogLevel.L_INFO, "after AsyncSend() to BB2\n");

    } else if (pkt.type() == 10004) {
        // 从BB2回来的应答报文
        SLOG(LogLevel.L_INFO, "input Message is type=%d\n", pkt.type());
        BFToBB2Rsp inner_rsp;
        inner_rsp = pkt.GetExtension(bf_to_bb2_rsp);
        BFLocalData *ld = ld_map_[pkt.seq()];

        for (int i = 0; i < inner_rsp.user_profile_size(); i++) {
            BFLocalData::UserProfile new_user_profile;
            UserProfile *user_profile_in_pkt = inner_rsp.user_profile(i);

            new_user_profile.gender_ = user_profile_in_pkt->gender();
            new_user_profile.user_name_ = user_profile_in_pkt->user_name();
            new_user_profile.user_desc_ = user_profile_in_pkt->user_desc();
            new_user_profile.user_id_ = user_profile_in_pkt->user_id();
            ld->userprofile_list_.push_back(new_user_profile);
        }

        // 构造发给C的响应报文
        PacketFormat rsp;
        CToBFRsp inner_c_to_bf_rsp;

        list<BFLocalData::UserProfile>::iterator it, endit = ld->userprofile_list_.end();

        for (it = ld->userprofile_list_.begin(); it != endit; it++) {
            UserProfile *tmp = inner_rsp.add_user_profile();
            tmp->set_user_id(it->user_id());
            tmp->set_gender(it->gender());
            tmp->set_user_name(it->user_name());
            tmp->set_user_desc(it->user_desc());
        }

        rsp.SetExtension(bf_to_bb2_req, inner_c_to_bf_rsp);
        req.set_seq(ld->seq_from_client_);
        req.set_service_id(1001);
        req.set_type(10002);
        req.set_version(1001);
        req.set_length(req.ByteSize());

        MemBlock *to_send = 0;
        int ret = MemPool::GetMemPool()->GetMemBlock(req.length(), to_send);
        rsp.SerializeToArray(to_send->start_, to_send->used_);

        string to_ip = ld->client_ipstr_;
        uint16_t to_port = ld->client_port_;

        client->AsyncSend(to_ip, to_port, "127.0.0.1", 0,
                        to_send, Socket::SocketType.T_TCP_SERVER,
                        Socket::DataFormat.DF_BIN, 0);
        SLOG(LogLevel.L_INFO, "after AsyncSend() to C\n");
        ld_map_.remove(ld->my_seq_);
    }
    LEAVING;
    return 0;
}
