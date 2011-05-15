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

#include "client_skeleton.hpp"

class TestClient : public ClientSkeleton {
public:
    boost::shared_ptr<MessageInfo> PrepareRequest(int type) {

        boost::shared_ptr<MessageInfo> msg(new MessageInfo);
        msg->set_from_ip("127.0.0.1");
        msg->set_from_port(0);
        msg->set_to_ip("127.0.0.1");
        msg->set_to_port(20001);

        msg->set_data(boost_shared_ptr<std::vector<char> >(new std::vector<char>));
        BFReq req;
        static int seq = 0;
        static user_id_ = 2;
        req.cmd_ = CMD_GET_USER_INFO;
        req.seq_ = seq++;
        req.user_id_ = user_id++;

        char *tmp_begin, *tmp_end;
        tmp_begin = reinterpret_cast<char*>(&req);
        tmp_end = tmp_begin + sizeof(BB2Rsp);
        msg->data()->assign(tmp_begin, tmp_end);
        return msg;
    };


};


int main(int argc, char const * const * const argv) {

    TestClient cli(2);

    cli.Run();
    return 0;
}
