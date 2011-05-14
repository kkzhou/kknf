
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

#ifndef _TEST_BB1_CONNECTION_HPP_
#define _TEST_BB1_CONNECTION_HPP_

#include "bin_connection.hpp"
#include "test_messages.hpp"


class TestBFConnection : public BinConnection {
public:
    TestBFConnection()
        : bb1_rsp_ready_(false),
        bb2_rsp_reay_(false){
    };

    static TestBFConnection* CreateTestBFConnection() {
        return new TestBFConnection;
    };

    // 业务逻辑相关的报文处理函数
    // 根据具体业务来实现
    // 返回值是一个列表，列表的元素是要发送的MessageInfo和对应的ConnectionFacotry对。
    // 含义是：在处理这个报文时，需要发出一些报文，为了发出这些报文，需要对应的connection，
    // 而这些connection就是用ConnectionFacotry建立。
    virtual ProcessResult& ProcessMessage(boost::shared_ptr<MessageInfo> msg) {

        Packet *orig_pkt = reinterpret_cast<BFReq*>(&(msg->data()->at(4)));
        if (orig_pkt->type_ == TYPE_BF_REQ) {
            BFReq req, *orig_ptr;
            orig_ptr = reinterpret_cast<BFReq*>(orig_pkt);
            req.seq_ = orig_ptr->seq_;
            req.user_id_ = orig_ptr->user_id_;
            req.cmd_ = orig_ptr->cmd_;

            boost::shared_ptr<MessageInfo> msg_to_bb1(new MessageInfo);
            boost::shared_ptr<MessageInfo> msg_to_bb2(new MessageInfo);

            msg_to_bb1->set_from_ip(msg->to_ip());
            msg_to_bb1->set_from_port(msg->to_port());

            std::string bb1_ip = "127.0.0.1";
            uint16_t bb1_port = 30001;
            msg_to_bb1->set_to_ip(bb1_ip);
            msg_to_bb1->set_to_port(bb1_port);

            msg_to_bb1->set_data(boost_shared_ptr<std::vector<char> >(new std::vector<char>));
            BB1Req bb1_req;
            bb1_req.cmd_ = req.cmd_;
            bb1_req.seq_ = req.seq_;
            bb1_req.user_id_ = req.user_id_;

            char *tmp_begin, *tmp_end;
            tmp_begin = reinterpret_cast<char*>(&bb1_req);
            tmp_end = tmp_begin + sizeof(BB1Req);
            msg_to_bb1->data()->assign(tmp_begin, tmp_end);

            msg_to_bb2->set_from_ip(msg->to_ip());
            msg_to_bb2->set_from_port(msg->to_port());

            std::string bb1_ip = "127.0.0.1";
            uint16_t bb1_port = 30002;
            msg_to_bb2->set_to_ip(bb1_ip);
            msg_to_bb2->set_to_port(bb1_port);

            msg_to_bb2->set_data(boost_shared_ptr<std::vector<char> >(new std::vector<char>));
            BB2Req bb2_req;
            bb2_req.cmd_ = req.cmd_;
            bb2_req.seq_ = req.seq_;
            bb2_req.user_id_ = req.user_id_;

            char *tmp_begin, *tmp_end;
            tmp_begin = reinterpret_cast<char*>(&bb2_req);
            tmp_end = tmp_begin + sizeof(BB2Req);
            msg_to_bb2->data()->assign(tmp_begin, tmp_end);

            ProcessResult ret;
            boost::functor<BasicConnection*()> connection_factory = CreateTestBFConnection;
            ret.push_back(std::pair(msg_to_bb1, connection_factory));
            ret.push_back(std::pair(msg_to_bb1, connection_factory));
            return ret;

        } else if (orig_pkt->type_ == TYPE_BB1_RSP) {
            BB1Rsp bb1_rsp, *orig_ptr;
            orig_ptr = reinterpret_cast<BB1Rsp*>(orig_pkt);
            bb1_rsp.seq_ = orig_ptr->seq_;
            bb1_rsp.user_id_ = orig_ptr->user_id_;
            bb1_rsp.cmd_ = orig_ptr->cmd_;
            bb1_rsp.gender_ = orig_ptr->gender_;
            bb1_rsp.type_ = orig_ptr->type_;

            rsp_to_client_.gender_ = bb1_rsp.gender_;
            rsp_to_client_.user_id_ = bb1_rsp.user_id_;
            if (bb2_rsp_reay_){
                // 发送返回给客户端的报文
            }
        } else if (orig_pkt->type_ == TYPE_BB2_RSP) {
        } else {
        }

    };
private:
    BFRsp rsp_to_client_;
    bool bb1_rsp_ready_;
    bool bb2_rsp_reay_;

};
#endif

