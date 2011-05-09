#ifndef _TEST_BF_CONNECTION_HPP_
#define _TEST_BF_CONNECTION_HPP_

#include <iostream>
#include "bin_connection.hpp"
#include "test_messages.hpp"

using namespace std;
using namespace AANF;

class TestBFConnection : public BinConnection  {
public:
    typedef boost::asio::ip::address IP_Address;
    typedef boost::asio::ip::tcp::socket TCP_Socket;
    typedef boost::asio::ip::tcp::endpoint TCP_Endpoint;
public:
    int ProcessMessage(boost::shared_ptr<MessageInfo> msg) {
        boost::shared_ptr<std::vector<char> > data = msg->data();
        BFReq *req = 0;
        req = &(data->at(4));
        cout << "Recv BFReq : userid=" << req->user_id_ << " seq=" << req->seq_ << " cmd=" << req->cmd_ << endl;
        std::string bb1_ip = "127.0.0.1";
        uint16_t bb1_port = 30001;
        IP_Address bb1_addr = boost::asio::ip::address::from_string(bb1_ip);
        TCP_Endpoint bb1(bb1_addr, bb1_port);

    };
private:
};
#endif
