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
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include <map>
#include <string>

#include "server.hpp"
#include "test_simple_packet.hpp"

using namespace AANF;
using namespace std;

class TestBB1 : public Server {
public:
    TestBB1()
        : dist(1, 1000000),
        die(gen, dist)
        {

    };
    // Process data
    int ProcessData(std::vector<char> &input_data, std::string &from_ip, uint16_t from_port,
                    std::string &to_ip, uint16_t to_port, PTime arrive_time) {

        vector<char> send_buf;

        TestPacketBase *baseptr = reinterpret_cast<TestPacketBase*>(&input_data[0]);

        if (baseptr->type_ != TestPacketBase::T_REQ_TO_BB1) {
            cerr << "Not supported packet type: " << baseptr->type_ << endl;
            return 0;
        }
        // request from bf
        ReqToBB1 *req_to_bb1 = reinterpret_cast<ReqToBB1*>(baseptr);
        cerr << "Recieve ReqToBB1.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(req_to_bb1->len_)
                << "ReqToBB1.type_ = " << req_to_bb1->type_
                << "ReqToBB1.seq_ = " << req_to_bb1->seq_
                << "ReqToBB1.a_ = " << req_to_bb1->a_ << endl;

        RspFromBB1 rsp;
        rsp.another_a_ = req_to_bb1->a_ +  die();
        rsp.seq_ = req_to_bb1->seq_;
        rsp.type_ = TestPacketBase::T_RSP_FROM_BB1;

        char *tmp = reinterpret_cast<char*>(&rsp);
        send_buf.assign(tmp, tmp + sizeof(RspFromBB1));

        SocketKey key(from_ip, from_port);
        SocketInfoPtr skinfo = FindIdleTCPClientSocket(key);
        if (!skinfo) {
            cerr << "Socket not found: ip=" << from_ip << " port=" << from_port << endl;
            return 0;
        }

        ToWriteThenRead(skinfo, send_buf);


        cerr << "Send RspFromBB1.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(rsp.len_)
            << "RspFromBB1.type_ = " << rsp.type_
            << "RspFromBB1.seq_ = " << rsp.seq_
            << "RspFromBB1.another_a_ = " << rsp.another_a_ << endl;

        return 0;
    };
public:
    string local_ip_;
    uint16_t port_low_;
    uint16_t port_high_;

private:
    boost::mt19937 gen;
    boost::uniform_int<> dist;
    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die;
};

int main(int argc, char **argv) {

    std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    TestBB1 bb1;
    int timer_interval = 10000;
    int thread_num = 4;

    int oc;
    int option_num = 0;
    const char *helpstr = " USAGE: ./test_bb1 -n threadnum -i timerinterval -L listenip -p listenport_low -P listenport_high -h";
    while ((oc = getopt(argc, argv, "i:L:n:p:P:h")) != -1) {
        switch (oc) {
            case 'i':
                timer_interval = atoi( optarg );
                break;
            case 'n':
                thread_num = atoi( optarg );
                break;
            case 'L':
                bb1.local_ip_ = optarg;
                option_num++;
                break;
            case 'p':
                bb1.port_low_ = atoi(optarg);
                option_num++;
                break;
            case 'P':
                bb1.port_high_ = atoi(optarg);
                option_num++;
                break;
            case 'h':
                cout << helpstr << endl;
                return 0;
            default:
                cout << helpstr << endl;
                return -1;
        }// switch
    } // while

    if (option_num < 3) {
        cout << helpstr << endl;
        return -1;
    }
    if (timer_interval < 0 || timer_interval > 100000) {
        cerr << "Parameter invalid: timerinterval is in [0, 100000]" << endl;
        return -1;
    }
    if (thread_num <= 0 || thread_num > 10000) {
        cerr << "Parameter invalid: threadnum is in [1, 10000]" << endl;
        return -1;
    }
    if (bb1.port_high_ < bb1.port_low_) {
        cerr << "Parameter invalid: listenport_low must be less than listenport_high" << endl;
        return -1;
    }
    bb1.set_timer_trigger_interval(timer_interval);
    bb1.set_thread_pool_size(thread_num);

    for (uint16_t port = bb1.port_low_; port <= bb1.port_high_; port++) {
        bb1.AddTCPAcceptor(bb1.local_ip_, port, SocketInfo::T_TCP_LV);
    }

    bb1.Run();
    std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    return 0;

};
