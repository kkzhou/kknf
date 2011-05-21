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
        bb2_rsp_arrived_(false) {
    };
public:
    bool bb1_rsp_arrived_;
    bool bb2_rsp_arrived_;
};


class TestBF : public Server {
public:
    // Process data
    int ProcessData(std::vector<char> &input_data, std::string &from_ip, uint16_t from_port
                    std::string &to_ip, uint16_t to_port, PTime arrive_time) {

        TestPacketBase *baseptr = reinterpret_cast<TestPacketBase*>(&input_data[0]);
        if (baseptr->type_ == TestPacketBase::PacketType.T_REQ_TO_BF) {
            ReqToBF *req_to_bf = reinterpret_cast<ReqToBF*>(baseptr);
            ReqToBB1 req1;
            req1.a_ = req_to_bf->a_;
            req1.seq_ = req_to_bf->seq_;
            req1.type_ = TestPacketBase::PacketType.T_REQ_TO_BB1;
        } else if (baseptr->type_ == TestPacketBase::PacketType.T_RSP_FROM_BB1) {
        } else if (baseptr->type_ == TestPacketBase::PacketType.T_RSP_FROM_BB1) {
        } else {
        }
        return 0;
    };
public:
    string local_ip_;
    uint16_t port_low_;
    uint16_t port_high_;
    map<int, ReqLocalData> ld_;
};

int main(int argc, char **argv) {

    std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    TestBF bf;
    int timer_interval = 10000;
    int thread_num = 4;

    int oc;
    const char *helpstr = " USAGE: ./test_bf -n threadnum -i timerinterval -I listenip -p listenport_low -P listenport_high -h";
    while ((oc = getopt(argc, argv, "i:I:n:p:h")) != -1) {
        switch (oc) {
            case 'i':
                timer_interval = atoi( optarg );
                break;
            case 'I':
                bf.local_ip_ = optarg;
                break;
            case 'p':
                bf.port_low_ = atoi( optarg );
                break;
            case 'P':
                bf.port_high_ = atoi( optarg );
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
        bf.AddAcceptor(bf.local_ip_, port, SocketType.T_TCP_LV);
    }

    bf.Run();
    std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    return 0;

};
