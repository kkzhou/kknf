#ifndef _TEST_BB_CONNECTION_HPP_
#define _TEST_BB_CONNECTION_HPP_

#include "bin_connection.hpp"


class TestBBConnection : public BinConnection {
public:
    class TestC2BFReq {
        int user_id_;
        int seq_;
    };
    // 业务逻辑相关的报文处理函数
    // 根据具体业务来实现
    // return value:
    // 0: OK
    // -1: Fail
    // 1: Process again
    int ProcessMessage(boost::shared_ptr<MessageInfo> msg) {

    };
private:

};
#endif
