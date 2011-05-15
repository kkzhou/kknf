
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

#include "bin_connection.hpp"
#include "test_messages.hpp"

#define TYPE_CLIENT_TO_BF_CONNECTION 0x00000001
#define TYPE_BF_TO_BB1_CONNECTION 0x00000002
#define TYPE_BF_TO_BB2_CONNECTION 0x00000003

class BFDataPerRequest : public DataPerRequest {
public:
    BFDataPerRequest(int req_index)
        : DataPerRequest(req_index),
        msg_to_client_(new MessageInfo) {
    };
public:
    bool bb1_rsp_arrived_;
    bool bb2_rsp_arrived_;
    BFRsp rsp_to_client_;
    boost::shared_ptr<MessageInfo> msg_to_client_;
};

class BFConnectionLocalData : public LocalData {
public :
    int connection_type_;
};

class TestBFConnection : public BinConnection {
public:
    TestBFConnection(boost::asio::io_service &io_serv, uint32_t init_recv_buffer_size,
                    uint32_t max_recv_buffer_size)
                    : ld_(new BFConnectionLocalData){
    };

    static BasicConnection* CreateTestBFConnection1(boost::asio::io_service &io_serv,
                    uint32_t init_recv_buffer_size, uint32_t max_recv_buffer_size) {

        TestBFConnection new_connection =
            new TestBFConnection(io_serv, init_recv_buffer_size, max_recv_buffer_size);

        boost::shared_ptr<BFConnectionLocalData> tmp_ld =
            boost::dynamic_pointer_cast<LocalData>(new_connection->ld());
        tmp_ld->connection_type_ = TYPE_CLIENT_TO_BF_CONNECTION;

        return new_connection;
    };

    static BasicConnection* CreateTestBFConnection2(boost::asio::io_service &io_serv,
                    uint32_t init_recv_buffer_size, uint32_t max_recv_buffer_size) {

        TestBFConnection new_connection =
            new TestBFConnection(io_serv, init_recv_buffer_size, max_recv_buffer_size);

        boost::shared_ptr<BFConnectionLocalData> tmp_ld =
            boost::dynamic_pointer_cast<LocalData>(new_connection->ld());
        tmp_ld->connection_type_ = TYPE_BF_TO_BB1_CONNECTION;

        return new_connection;
    };
    static BasicConnection* CreateTestBFConnection3(boost::asio::io_service &io_serv,
                    uint32_t init_recv_buffer_size, uint32_t max_recv_buffer_size) {

        TestBFConnection new_connection =
            new TestBFConnection(io_serv, init_recv_buffer_size, max_recv_buffer_size);

        boost::shared_ptr<BFConnectionLocalData> tmp_ld =
            boost::dynamic_pointer_cast<LocalData>(new_connection->ld());
        tmp_ld->connection_type_ = TYPE_BF_TO_BB2_CONNECTION;

        return new_connection;
    };

    // 业务逻辑相关的报文处理函数
    // 根据具体业务来实现
    // 返回值是一个列表，列表的元素是要发送的MessageInfo和对应的ConnectionFacotry对。
    // 含义是：在处理这个报文时，需要发出一些报文，为了发出这些报文，需要对应的connection，
    // 而这些connection就是用ConnectionFacotry建立。
    virtual ProcessResult& ProcessMessage(boost::shared_ptr<MessageInfo> msg) {

        Packet *orig_pkt = reinterpret_cast<BFReq*>(&(msg->data()->at(4)));
        int connection_type;
        boost::shared_ptr<BFConnectionLocalData> ld =
            boost::dynamic_pointer_cast<LocalData>(new_connection->ld());
        connection_type = ld->connection_type_;

        static int req_index = 0;

        boost::shared_ptr<BFDataPerRequest> data_for_this_req;
        char *tmp_begin, *tmp_end;
        ProcessResult ret;
        if (connection_type == TYPE_CLIENT_TO_BF_CONNECTION) {
            // 这是从客户端（相对的客户端，不一定是用户）过来的新Request，
            // 创建一个新的DataPerRequest放到map里
            int req_index_for_this_req = ++req_index;
            data_for_this_req
                = boost::shared_ptr<BFDataPerRequest>(new BFDataPerRequest(req_index_for_this_req);

            data_for_this_req->msg_to_client_->set_from_ip(msg->to_ip());
            data_for_this_req->msg_to_client_->set_from_port(msg->to_port());
            data_for_this_req->msg_to_client_->set_to_ip(msg->from_ip());
            data_for_this_req->msg_to_client_->set_to_port(msg->from_port());

            ld->InsertDataPerRequest(req_index_for_this_req, data_for_this_req));

            BFReq req, *orig_ptr;
            orig_ptr = reinterpret_cast<BFReq*>(orig_pkt);
            req.seq_ = orig_ptr->seq_;
            req.user_id_ = orig_ptr->user_id_;
            req.cmd_ = orig_ptr->cmd_;
            data_for_this_req->rsp_to_client_.cmd_ = req.cmd_;
            data_for_this_req->rsp_to_client_.seq_ = req.seq_;

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
            bb1_req.req_index_ = req_index_for_this_req;

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
            bb2_req.req_index_ = req_index_for_this_req;


            tmp_begin = reinterpret_cast<char*>(&bb2_req);
            tmp_end = tmp_begin + sizeof(BB2Req);
            msg_to_bb2->data()->assign(tmp_begin, tmp_end);

            boost::functor<BasicConnection*()> connection_factory1 = CreateTestBFConnection2;
            boost::functor<BasicConnection*()> connection_factory2 = CreateTestBFConnection2;
            ret.push_back(std::pair(msg_to_bb1, connection_factory1));
            ret.push_back(std::pair(msg_to_bb2, connection_factory2));
            return ret;

        } else if (connection_type == TYPE_BF_TO_BB1_CONNECTION) {
            BB1Rsp bb1_rsp, *orig_ptr;
            orig_ptr = reinterpret_cast<BB1Rsp*>(orig_pkt);
            bb1_rsp.seq_ = orig_ptr->seq_;
            bb1_rsp.user_id_ = orig_ptr->user_id_;
            bb1_rsp.cmd_ = orig_ptr->cmd_;
            bb1_rsp.gender_ = orig_ptr->gender_;
            bb1_rsp.type_ = orig_ptr->type_;
            bb1_rsp.req_index_ = orig_ptr->req_index_;

            data_for_this_req =
                boost::dynamic_pointer_cast<BFDataPerRequest>(ld()->GetDataPerRequest(bb1_rsp.req_index_));
            data_for_this_req->rsp_to_client_.gender_ = bb1_rsp.gender_;
            data_for_this_req->rsp_to_client_.user_id_ = bb1_rsp.user_id_;
            data_for_this_req->bb1_rsp_arrived_ = true;

        } else if (connection_type == TYPE_BF_TO_BB2_CONNECTION) {
            BB2Rsp bb2_rsp, *orig_ptr;
            orig_ptr = reinterpret_cast<BB2Rsp*>(orig_pkt);
            bb2_rsp.seq_ = orig_ptr->seq_;
            bb2_rsp.user_id_ = orig_ptr->user_id_;
            bb2_rsp.cmd_ = orig_ptr->cmd_;
            bb2_rsp.age_ = orig_ptr->age_;
            bb2_rsp.type_ = orig_ptr->type_;
            bb2_rsp.req_index_ = orig_ptr->req_index_;

            data_for_this_req =
                boost::dynamic_pointer_cast<BFDataPerRequest>(ld()->GetDataPerRequest(bb2_rsp.req_index_));
            data_for_this_req->rsp_to_client_.age_ = bb2_rsp.age_;
            data_for_this_req->rsp_to_client_.user_id_ = bb2_rsp.user_id_;
            data_for_this_req->bb2_rsp_arrived_ = true;

        } else {
        }

        // 如果BB1和BB2都给了应答，则发送回应给客户端
        data_for_this_req =
                boost::dynamic_pointer_cast<BFDataPerRequest>(ld()->GetDataPerRequest(bb2_rsp.req_index_));
        if (data_for_this_req->bb1_rsp_arrived_ && data_for_this_req->bb2_rsp_arrived_) {
            data_for_this_req->msg_to_client_->set_arrive_time(boost::posix_time::second_time::localtime());
            data_for_this_req->msg_to_client_->set_data(boost_shared_ptr<std::vector<char> >(new std::vector<char>));
            tmp_begin = reinterpret_cast<char*>(&data_for_this_req->rsp_to_client_);
            tmp_end = tmp_begin + sizeof(BFRsp);
            data_for_this_req->msg_to_client_->data().assgn();


            ret.push_back(std::pair(data_for_this_req->msg_to_client_, boost::functor<BasicConnection(*()>())));

        }
        return ret;
    };


};
#endif

