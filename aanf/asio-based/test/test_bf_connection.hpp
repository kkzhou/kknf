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
