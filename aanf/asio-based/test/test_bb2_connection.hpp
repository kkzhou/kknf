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

#ifndef _TEST_BB2_CONNECTION_HPP_
#define _TEST_BB2_CONNECTION_HPP_

#include "bin_connection.hpp"
#include "test_messages.hpp"


class TestBB2Connection : public BinConnection {
public:
    // 业务逻辑相关的报文处理函数
    // 根据具体业务来实现
    // 返回值是一个列表，列表的元素是要发送的MessageInfo和对应的ConnectionFacotry对。
    // 含义是：在处理这个报文时，需要发出一些报文，为了发出这些报文，需要对应的connection，
    // 而这些connection就是用ConnectionFacotry建立。
    virtual ProcessResult& ProcessMessage(boost::shared_ptr<MessageInfo> msg) {

        BB2Req req, *orig_ptr;
        orig_ptr = reinterpret_cast<BB2Req*>(&(msg->data()->at(4)));
        req.seq_ = orig_ptr->seq_;
        req.user_id_ = orig_ptr->user_id_;
        req.cmd_ = orig_ptr->cmd_;

        boost::shared_ptr<MessageInfo> return_msg(new MessageInfo);
        return_msg->set_from_ip(msg->to_ip());
        return_msg->set_from_port(msg->to_port());
        return_msg->set_to_ip(msg->from_ip());
        return_msg->set_to_port(msg->from_port());

        return_msg->set_data(boost_shared_ptr<std::vector<char> >(new std::vector<char>));
        BB2Rsp rsp;
        rsp.cmd_ = req.cmd_;
        rsp.seq_ = req.seq_;
        rsp.user_id_ = req.user_id_;
        static int age = 0;
        rsp.age_ = age++;

        char *tmp_begin, *tmp_end;
        tmp_begin = reinterpret_cast<char*>(&rsp);
        tmp_end = tmp_begin + sizeof(BB2Rsp);
        return_msg->data()->assign(tmp_begin, tmp_end);

        ProcessResult ret;
        ret.push_back(return_msg, boost::functor<BasicConnection*()>());
        return ret;
    };

};
#endif
