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

#ifndef _TEST_CLIENT_CONNECTION_HPP_
#define _TEST_CLIENT_CONNECTION_HPP_

#include "bin_connection.hpp"
#include "test_messages.hpp"


class TestClientConnection : public BinConnection {
public:
    // 业务逻辑相关的报文处理函数
    // 根据具体业务来实现
    // 返回值是一个列表，列表的元素是要发送的MessageInfo和对应的ConnectionFacotry对。
    // 含义是：在处理这个报文时，需要发出一些报文，为了发出这些报文，需要对应的connection，
    // 而这些connection就是用ConnectionFacotry建立。
    virtual ProcessResult& ProcessMessage(boost::shared_ptr<MessageInfo> msg) {

        BFRsp rsp, *orig_ptr;
        orig_ptr = reinterpret_cast<BFRsp*>(&(msg->data()->at(4)));
        rsp.seq_ = orig_ptr->seq_;
        rsp.user_id_ = orig_ptr->user_id_;
        rsp.cmd_ = orig_ptr->cmd_;
        rsp.gender_ = orig_ptr->gender_;
        rsp.type_ = orig_ptr->type_;

        return ret;
    };

    static boost::shared_ptr<BasicConnection> CreateConnection(
                    boost::asio::io_service &io_serv,
                    uint32_t init_recv_buffer_size, uint32_t max_recv_buffer_size) = 0;
    {
        std::string my_ip = "127.0.0.1";
        uint16_t my_port = 0;
        boost::system::error_code ec;
        IP_Address my_addr = boost::asio::ip::address::from_string(my_ip, ec);
        if (ec == boost::system::errc.bad_address) {
            return -1;
        }

        TCP_Endpoint my_endpoint(my_addr, my_port);
        boost::shared_ptr<BasicConnect> new_connection(new TestClientConnection(io_service_, 1024, 10240));
        new_connection->socket()->bind(my_endpoint);
        return new_connection;
    };
};
#endif
