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

#include "skeleton.hpp"
#include "test_packet.hpp"

using namespace AANF;
using namespace std;

class TestBB2 : public Skeleton {
public:
    TestBB2()
        : dist(1, 1000000),
        die(gen, dist)
        {

    };
    virtual ~TestBB2(){};
    // Timer handler
    void PrintHeartBeat() {
       cerr << "I'am alive!" << endl;
    };
    // Process data
    virtual int ProcessData(std::vector<char> &input_data, std::string &from_ip, uint16_t from_port,
                    std::string &to_ip, uint16_t to_port, PTime arrive_time) {

        vector<char> send_buf;

        TestPacketBase *baseptr = reinterpret_cast<TestPacketBase*>(&input_data[0]);

        if (baseptr->type_ != TestPacketBase::T_REQ_TO_BB2) {
            cerr << "Not supported packet type: " << baseptr->type_ << endl;
            return -1;
        }
        // request from bf
        ReqToBB2 *req_to_bb2 = reinterpret_cast<ReqToBB2*>(baseptr);
        cerr << "Recieve ReqToBB2.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(req_to_bb2->len_)
                << " ReqToBB2.type_ = " << req_to_bb2->type_
                << " ReqToBB2.seq_ = " << req_to_bb2->seq_
                << " ReqToBB2.b_ = " << req_to_bb2->b_
                << " time = " << boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time()) << endl;

        RspFromBB2 rsp;
        rsp.another_b_ = req_to_bb2->b_ +  die();
        rsp.len_= boost::asio::detail::socket_ops::host_to_network_long(sizeof(RspFromBB1));
        rsp.seq_ = req_to_bb2->seq_;
        rsp.type_ = TestPacketBase::T_RSP_FROM_BB2;

        char *tmp = reinterpret_cast<char*>(&rsp);
        send_buf.assign(tmp, tmp + sizeof(RspFromBB2));

        IPAddress addr;
        boost::system::error_code e;
        addr = IPAddress::from_string(from_ip, e);
        UDPEndpoint endpoint(addr, from_port);
        UDPToWrite(endpoint, send_buf);

        cerr << "Send RspFromBB2.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(rsp.len_)
            << " RspFromBB2.type_ = " << rsp.type_
            << " RspFromBB2.seq_ = " << rsp.seq_
            << " RspFromBB2.another_b_ = " << rsp.another_b_
            << " time = " << boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time()) << endl;

        return 2;
    };
private:
    boost::mt19937 gen;
    boost::uniform_int<> dist;
    boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die;
};

int main(int argc, char **argv) {

    std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    boost::shared_ptr<TestBB2> bb2(new TestBB2);
    int timer_interval = 10000;
    string local_udp_ip;
    uint16_t local_udp_port;

    int oc;
    int option_num = 0;
    const char *helpstr = " USAGE: ./test_bb2 -n threadnum -i timerinterval -L listenip -p listenport -h";
    while ((oc = getopt(argc, argv, "i:L:n:p:P:h")) != -1) {
        switch (oc) {
            case 'i':
                timer_interval = atoi(optarg);
                break;
            case 'L':
                local_udp_ip = optarg;
                option_num++;
                break;
            case 'p':
                local_udp_port = atoi(optarg);
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

    bb2->set_timer_trigger_interval(timer_interval);

    IPAddress addr;
    boost::system::error_code e;
    addr = IPAddress::from_string(local_udp_ip, e);

    if (e) {
        cerr << "IP address format invalid: " << local_udp_ip << endl;
        exit(1);
    }
    bb2->InitUDPSocket(UDPEndpoint(addr, local_udp_port));
    bb2->AddTimerHandler(boost::bind(&TestBB2::PrintHeartBeat, bb2));
    bb2->Run();
    std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    return 0;

};
