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

class TestBB2 : public Server {
public:
    TestBB2()
        : dist(1, 1000000),
        die(gen, dist)
        {

    };
    // Timer handler
    void PrintHeartBeat() {
       cerr << "I'am alive!" << endl;
    };
    // Process data
    int ProcessData(std::vector<char> &input_data, std::string &from_ip, uint16_t from_port,
                    std::string &to_ip, uint16_t to_port, PTime arrive_time) {

        vector<char> send_buf;

        TestPacketBase *baseptr = reinterpret_cast<TestPacketBase*>(&input_data[0]);

        if (baseptr->type_ != TestPacketBase::T_REQ_TO_BB2) {
            cerr << "Not supported packet type: " << baseptr->type_ << endl;
            return 0;
        }
        // request from bf
        ReqToBB2 *req_to_bb2 = reinterpret_cast<ReqToBB2*>(baseptr);
        cerr << "Recieve ReqToBB2.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(req_to_bb2->len_)
                << "ReqToBB2.type_ = " << req_to_bb2->type_
                << "ReqToBB2.seq_ = " << req_to_bb2->seq_
                << "ReqToBB2.b_ = " << req_to_bb2->b_ << endl;

        RspFromBB2 rsp;
        rsp.another_b_ = req_to_bb2->b_ +  die();
        rsp.seq_ = req_to_bb2->seq_;
        rsp.type_ = TestPacketBase::T_RSP_FROM_BB2;

        char *tmp = reinterpret_cast<char*>(&req_to_bb2);
        send_buf.assign(tmp, tmp + sizeof(RspFromBB2));

        IPAddress addr;
        boost::system::error_code e;
        addr.from_string(from_ip, e);
        TCPEndpoint endpoint(addr, from_port);
        SocketInfoPtr skinfo = FindIdleTCPClientSocket(endpoint);
        if (!skinfo) {
            cerr << "Socket not found: ip=" << from_ip << " port=" << from_port << endl;
            return 0;
        }

        ToWriteThenRead(skinfo, send_buf);


        cerr << "Send RspFromBB2.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(rsp.len_)
            << "RspFromBB2.type_ = " << rsp.type_
            << "RspFromBB2.seq_ = " << rsp.seq_
            << "RspFromBB2.another_b_ = " << rsp.another_b_ << endl;

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
    boost::shared_ptr<TestBB2> bb2(new TestBB2);
    int timer_interval = 10000;

    int oc;
    int option_num = 0;
    const char *helpstr = " USAGE: ./test_bb1 -n threadnum -i timerinterval -L listenip -p listenport_low -P listenport_high -h";
    while ((oc = getopt(argc, argv, "i:L:n:p:P:h")) != -1) {
        switch (oc) {
            case 'i':
                timer_interval = atoi(optarg);
                break;
            case 'n':
                thread_num = atoi(optarg);
                break;
            case 'L':
                option_num++;
                bb2->local_ip_ = optarg;
                break;
            case 'p':
                option_num++;
                bb2->port_low_ = atoi(optarg);
                break;
            case 'P':
                option_num++;
                bb2->port_high_ = atoi(optarg);
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

    if (bb2->port_high_ < bb2->port_low_) {
        cerr << "Parameter invalid: listenport_low must be less than listenport_high" << endl;
        return -1;
    }
    bb2->set_timer_trigger_interval(timer_interval);

    IPAddress addr;
    boost::system::error_code e;
    addr.from_string(bb1->local_ip_, e);

    if (e) {
        cerr << "IP address format invalid: " << bb1->local_ip_ << endl;
        exit(1);
    }

    for (uint16_t port = bb2->port_low_; port <= bb2->port_high_; port++) {
        bb2->AddTCPAcceptor(TCPEndpoint(addr, port), SocketInfo::T_TCP_LV);
    }

    bb2->AddTimerHandler(boost::bind(&TestBB2::PrintHeartBeat, bb2));
    bb2->Run();
    std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    return 0;

};
