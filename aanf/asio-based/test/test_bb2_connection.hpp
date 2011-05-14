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
