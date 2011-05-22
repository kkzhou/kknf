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

#include "client.hpp"
#include "test_simple_packet.hpp"

using namespace AANF;
using namespace std;

class TestClient : public Client {
public:
    // Process data
    int ProcessData(std::vector<char> &input_data, std::string &from_ip, uint16_t from_port,
                    std::string &to_ip, uint16_t to_port, PTime arrive_time) {

       std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
       RspFromBF *rsp = reinterpret_cast<RspFromBF*>(&input_data[0]);
       cerr << "Recieve RspFromBF.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(rsp->len_)
            << "RspFromBF.type_ = " << rsp->type_
            << "RspFromBF.seq_ = " << rsp->seq_
            << "RspFromBF.sum_ = " << rsp->sum_
            << "RspFromBF.error_ = " << rsp->error_ << endl;

       std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    void PrepareDataThenSend() {
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

        SocketKey key(server_ip_, server_port_);
        SocketInfoPtr skinfo = FindIdleTCPClientSocket(key);
        if (!skinfo) {
            IPAddress addr = IPAddress::from_string(server_ip_);
            TCPEndpoint remote_endpoint(addr, server_port_);
            ToConnectThenWrite(remote_endpoint, send_buf);
        } else {
            ToWriteThenRead(skinfo, send_buf);
        }
        cerr << "Send ReqToBF.len_ = " << boost::asio::detail::socket_ops::network_to_host_long(req.len_)
            << "ReqToBF.type_ = " << req.type_
            << "ReqToBF.seq_ = " << req.seq_
            << "ReqToBF.a_ = " << req.a_
            << "ReqToBF.b_ = " << req.b_ << endl;

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };
public:
    string server_ip_;
    uint16_t server_port_;
};


int main(int argc, char **argv) {

    std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    boost::shared_ptr<TestClient> client(new TestClient);
    int timer_interval = 10000;
    int thread_num = 4;

    int oc;
    int option_num = 0;
    const char *helpstr = " USAGE: ./test_simple_client -n threadnum -i timerinterval -I bfip -p bfport -h";
    while ((oc = getopt(argc, argv, "i:I:n:p:h")) != -1) {
        switch (oc) {
            case 'i':
                timer_interval = atoi(optarg);
                break;
            case 'I':
                client->server_ip_ = optarg;
                option_num++;
                break;
            case 'p':
                client->server_port_ = atoi(optarg);
                option_num++;
                break;
            case 'n':
                thread_num = atoi(optarg);
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
    if (thread_num <= 0 || thread_num > 10000) {
        cerr << "Parameter invalid: threadnum is in [1, 10000]" << endl;
        return -1;
    }

    if (client->server_ip_.empty() || client->server_port_ == 0) {
        cout << helpstr << endl;
        return -1;
    }
    client->set_timer_trigger_interval(timer_interval);
    client->set_thread_pool_size(thread_num);
    client->AddTimerHandler(boost::bind(&Client::PrepareDataThenSend, client));

    client->Run();
    std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    return 0;

};
