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
#include <map>
#include <string>

#include "server.hpp"
#include "test_simple_packet.hpp"

using namespace AANF;
using namespace std;

class ReqLocalData {
public:
    ReqLocalData()
        :bb1_rsp_arrived_(false),
        bb2_rsp_arrived_(false),
        client_port_(0) {
    };
public:
    bool bb1_rsp_arrived_;
    bool bb2_rsp_arrived_;
    RspFromBF rsp_;
    string client_ip_;
    uint16_t client_port_;
};


class TestBF : public Server {
public:
    // Process data
    int ProcessData(std::vector<char> &input_data, std::string &from_ip, uint16_t from_port,
                    std::string &to_ip, uint16_t to_port, PTime arrive_time) {

        vector<char> send_buf1, send_buf2, send_buf3;

        TestPacketBase *baseptr = reinterpret_cast<TestPacketBase*>(&input_data[0]);

        if (baseptr->type_ == TestPacketBase::T_REQ_TO_BF) {
            // request from client
            ReqToBF *req_to_bf = reinterpret_cast<ReqToBF*>(baseptr);
            cerr << "Recieve ReqToBF.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(req_to_bf->len_)
                << "ReqToBF.type_ = " << req_to_bf->type_
                << "ReqToBF.seq_ = " << req_to_bf->seq_
                << "ReqToBF.a_ = " << req_to_bf->a_
                << "ReqToBF.b_ = " << req_to_bf->b_ << endl;

            map<int, ReqLocalData>::iterator it = ld_.find(req_to_bf->seq_);
            if (it != ld_.end()) {
                cerr << "Seq error: seqnum(" << req_to_bf->seq_ << ") is taken." << endl;
                return 0;
            }

            ReqLocalData ld_for_this_req;

            ReqToBB1 req1;
            req1.a_ = req_to_bf->a_;
            req1.seq_ = req_to_bf->seq_;
            req1.type_ = TestPacketBase::T_REQ_TO_BB1;

            ld_for_this_req.rsp_.error_ = 0;
            ld_for_this_req.client_ip_ = from_ip;
            ld_for_this_req.client_port_ = from_port;
            ld_for_this_req.rsp_.len_ = boost::asio::detail::socket_ops::network_to_host_long(sizeof(RspFromBF));
            ld_for_this_req.rsp_.seq_ = req_to_bf->seq_;
            ld_for_this_req.rsp_.type_ = TestPacketBase::T_RSP_FROM_BF;
            ld_.insert(pair<int, ReqLocalData>(req_to_bf->seq_, ld_for_this_req));

            char *tmp = reinterpret_cast<char*>(&req1);
            send_buf1.assign(tmp, tmp + sizeof(ReqToBB1));

            SocketKey key1(bb1_ip_, bb1_port_);
            SocketInfoPtr skinfo1 = FindIdleTCPClientSocket(key1);
            if (!skinfo1) {
                IPAddress addr = IPAddress::from_string(bb1_ip_);
                TCPEndpoint remote_endpoint(addr, bb1_port_);
                ToConnectThenWrite(remote_endpoint, send_buf1);
            } else {
                ToWriteThenRead(skinfo1, send_buf1);
            }

            cerr << "Send ReqToBB1.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(req1.len_)
                << "ReqToBB1.type_ = " << req1.type_
                << "ReqToBB1.seq_ = " << req1.seq_
                << "ReqToBB1.a_ = " << req1.a_ << endl;

            ReqToBB2 req2;
            req2.b_ = req_to_bf->b_;
            req2.seq_ = req_to_bf->seq_;
            req2.type_ = TestPacketBase::T_REQ_TO_BB2;

            tmp = reinterpret_cast<char*>(&req2);
            send_buf2.assign(tmp, tmp + sizeof(ReqToBB2));

            SocketKey key2(bb2_ip_, bb2_port_);
            SocketInfoPtr skinfo2 = FindIdleTCPClientSocket(key2);
            if (!skinfo2) {
                IPAddress addr = IPAddress::from_string(bb2_ip_);
                TCPEndpoint remote_endpoint(addr, bb2_port_);
                ToConnectThenWrite(remote_endpoint, send_buf2);
            } else {
                ToWriteThenRead(skinfo2, send_buf2);
            }
            cerr << "Send ReqToBB2.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(req2.len_)
                << "ReqToBB2.type_ = " << req2.type_
                << "ReqToBB2.seq_ = " << req2.seq_
                << "ReqToBB2.b_ = " << req2.b_ << endl;
            return 0;

        } else if (baseptr->type_ == TestPacketBase::T_RSP_FROM_BB1) {
            // rsp form bb1
            RspFromBB1 *rsp_from_bb1 = reinterpret_cast<RspFromBB1*>(baseptr);
            cerr << "Recieve RspFromBB1.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(rsp_from_bb1->len_)
                << "RspFromBB1.type_ = " << rsp_from_bb1->type_
                << "RspFromBB1.seq_ = " << rsp_from_bb1->seq_
                << "RspFromBB1.another_a_ = " << rsp_from_bb1->another_a_ << endl;

            map<int, ReqLocalData>::iterator it = ld_.find(rsp_from_bb1->seq_);
            if (it == ld_.end()) {
                cerr << "Seq error: seqnum(" << rsp_from_bb1->seq_ << ") doesn't exist." << endl;
                return 0;
            }

            it->second.rsp_.sum_ += rsp_from_bb1->another_a_;
            it->second.bb1_rsp_arrived_= true;

        } else if (baseptr->type_ == TestPacketBase::T_RSP_FROM_BB2) {
            // rsp form bb2
            RspFromBB2 *rsp_from_bb2 = reinterpret_cast<RspFromBB2*>(baseptr);
            cerr << "Recieve RspFromBB2.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(rsp_from_bb2->len_)
                << "RspFromBB2.type_ = " << rsp_from_bb2->type_
                << "RspFromBB2.seq_ = " << rsp_from_bb2->seq_
                << "RspFromBB2.another_b_ = " << rsp_from_bb2->another_b_ << endl;

            map<int, ReqLocalData>::iterator it = ld_.find(rsp_from_bb2->seq_);
            if (it == ld_.end()) {
                cerr << "Seq error: seqnum(" << rsp_from_bb2->seq_ << ") doesn't exist." << endl;
                return 0;
            }

            it->second.rsp_.sum_ += rsp_from_bb2->another_b_;
            it->second.bb2_rsp_arrived_= true;
        } else {
            cerr << "Not supported packet type: " << baseptr->type_ << endl;
            return -1;
        }

        map<int, ReqLocalData>::iterator tmpit = ld_.find(baseptr->seq_);
        if (tmpit != ld_.end()) {
            cerr << "Seq error: seqnum(" << baseptr->seq_ << ") is taken." << endl;
            return 0;
        }
        if (tmpit->second.bb1_rsp_arrived_ && tmpit->second.bb2_rsp_arrived_) {
            // build rsp to client
            char *tmp = reinterpret_cast<char*>(&tmpit->second.rsp_);
            send_buf3.assign(tmp, tmp + sizeof(RspFromBF));

            SocketKey key3(tmpit->second.client_ip_, tmpit->second.client_port_);
            SocketInfoPtr skinfo3 = FindIdleTCPClientSocket(key3);
            if (!skinfo3) {
                IPAddress addr = IPAddress::from_string(tmpit->second.client_ip_);
                TCPEndpoint remote_endpoint(addr, tmpit->second.client_port_);
                ToConnectThenWrite(remote_endpoint, send_buf3);
            } else {
                ToWriteThenRead(skinfo3, send_buf3);
            }

            cerr << "Send RspFromBF.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(tmpit->second.rsp_.len_)
                << "RspFromBF.type_ = " << tmpit->second.rsp_.type_
                << "RspFromBF.seq_ = " << tmpit->second.rsp_.seq_
                << "RspFromBF.sum_ = " << tmpit->second.rsp_.sum_ << endl;
        }
        return 0;
    };
public:
    string local_ip_;
    uint16_t port_low_;
    uint16_t port_high_;
    map<int, ReqLocalData> ld_;
    string bb1_ip_;
    uint16_t bb1_port_;
    string bb2_ip_;
    uint16_t bb2_port_;
};

int main(int argc, char **argv) {

    std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    TestBF bf;
    int timer_interval = 10000;
    int thread_num = 4;

    int oc;
    const char *helpstr = " USAGE: ./test_bf -n threadnum -i timerinterval -L listenip -p listenport_low -P listenport_high -a bb1ip -b bbiport -c bb2ip -d bb2port -h";
    while ((oc = getopt(argc, argv, "i:I:n:p:h")) != -1) {
        switch (oc) {
            case 'i':
                timer_interval = atoi( optarg );
                break;
            case 'L':
                bf.local_ip_ = optarg;
                break;
            case 'p':
                bf.port_low_ = atoi(optarg);
                break;
            case 'P':
                bf.port_high_ = atoi(optarg);
                break;
            case 'a':
                bf.bb1_ip_ = atoi(optarg);
                break;
            case 'b':
                bf.bb1_port_ = atoi(optarg);
                break;
            case 'c':
                bf.bb2_ip_ = atoi(optarg);
                break;
            case 'd':
                bf.bb2_port_ = atoi(optarg);
                break;
            case 'h':
                cout << helpstr << endl;
                return 0;
            default:
                cout << helpstr << endl;
                return -1;
        }// switch
    } // while

    if (timer_interval < 0 || timer_interval > 100000) {
        cerr << "Parameter invalid: timerinterval is in [0, 100000]" << endl;
        return -1;
    }
    if (thread_num <= 0 || thread_num > 10000) {
        cerr << "Parameter invalid: threadnum is in [1, 10000]" << endl;
        return -1;
    }
    if (bf.port_high_ < bf.port_low_) {
        cerr << "Parameter invalid: listenport_low must be less than listenport_high" << endl;
        return -1;
    }
    bf.set_timer_trigger_interval(timer_interval);
    bf.set_thread_pool_size(thread_num);

    for (uint16_t port = bf.port_low_; port <= bf.port_high_; port++) {
        bf.AddTCPAcceptor(bf.local_ip_, port, SocketInfo::T_TCP_LV);
    }

    bf.Run();
    std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    return 0;

};
