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

#include "server_skeleton.hpp"

class BB2Server : public ServerSkeleton {
public:
    int Init(std::string &configfile) {
        // 如果是复杂的初始化参数，一般从配置文件里读
        return 0;

    };
private:

};

int main(int argc, char const * const * const argv) {

    BB1Server srv(2);
    srv.AddListenSocket("127.0.0.1", 30001, 10, TestBB2Connection::CreateTestBB2Connection);
    srv.AddListenSocket("127.0.0.1", 30002, 10, TestBB2Connection::CreateTestBB2Connection);

    srv.Run();
    return 0;
}
