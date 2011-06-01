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

#include <iostream>

#include "skeleton.hpp"
#include "test_packet.hpp"

using namespace AANF;
using namespace std;

class TestClient : public Skeleton {
public:
    TestClient(){};
    virtual ~TestClient(){};
    // Process data
    virtual int ProcessData(std::vector<char> &input_data, std::string &from_ip, uint16_t from_port,
                    std::string &to_ip, uint16_t to_port, PTime arrive_time) {

       std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
       RspFromBF *rsp = reinterpret_cast<RspFromBF*>(&input_data[0]);
       cerr << "Recieve RspFromBF.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(rsp->len_)
            << " RspFromBF.type_ = " << rsp->type_
            << " RspFromBF.seq_ = " << rsp->seq_
            << " RspFromBF.sum_ = " << rsp->sum_
            << " RspFromBF.error_ = " << rsp->error_
            << " time = " << boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time()) << endl;

       std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
       return 1;
    };

    virtual void PrepareDataThenSend() {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        static int a = 0;
        static int b = -1;
        static int seq = 1;

        ReqToBF req;
        req.type_ = TestPacketBase::T_REQ_TO_BF;
        req.a_ = a++;
        req.b_ = b--;
        req.seq_ = seq++;
        int len = sizeof(ReqToBF);
        req.len_ = boost::asio::detail::socket_ops::network_to_host_long(len);


        vector<char> send_buf;


        char *tmp = reinterpret_cast<char*>(&req);
        send_buf.assign(tmp, tmp + len);

        cerr << "To find an idle Socket." << endl;
        SocketInfoPtr skinfo = FindIdleTCPClientSocket(server_endpoint_);
        if (!skinfo) {
            cerr << "No idle Socket, to create a new one." << endl;
            ToConnectThenWrite(server_endpoint_, SocketInfo::T_TCP_LV, send_buf);
        } else {
            cerr << "Idle Socket found." << endl;
            ToWriteThenRead(skinfo, send_buf);
        }

        cerr << "Send ReqToBF.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(req.len_)
            << " ReqToBF.type_ = " << req.type_
            << " ReqToBF.seq_ = " << req.seq_
            << " ReqToBF.a_ = " << req.a_
            << " ReqToBF.b_ = " << req.b_
            << " time = " << boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time()) << endl;

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };
public:
    TCPEndpoint server_endpoint_;
};


int main(int argc, char **argv) {

    std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    boost::shared_ptr<TestClient> client(new TestClient);
    int timer_interval = 10000;
    IPAddress addr;
    uint16_t port;

    int oc;
    int option_num = 0;
    boost::system::error_code e;
    const char *helpstr = " USAGE: ./test_simple_client -i timerinterval -I bfip -p bfport -h";
    while ((oc = getopt(argc, argv, "i:I:p:h")) != -1) {
        switch (oc) {
            case 'i':
                timer_interval = atoi(optarg);
                break;
            case 'I':
                addr = IPAddress::from_string(optarg, e);
                option_num++;
                break;
            case 'p':
                port = atoi(optarg);
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

    if (option_num < 2) {
        cout << helpstr << endl;
        return -1;
    }
    if (timer_interval < 0 || timer_interval > 100000) {
        cerr << "Parameter invalid: timerinterval is in [0, 100000]" << endl;
        return -1;
    }

    if (e) {
        cerr << "IP address format invalid: " << e.message();
    }

    client->set_timer_trigger_interval(timer_interval);

    client->server_endpoint_ = TCPEndpoint(addr, port);
    client->AddTimerHandler(boost::bind(&TestClient::PrepareDataThenSend, client));

    client->Run();
    std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    return 0;

};
