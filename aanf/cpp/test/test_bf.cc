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

#include <string>
#include <iostream>
#include "server.h"

using std;
using namespace AANF;

class TestBF : public Skeleton {
public:
    virtual int ProcessPacket(Packet &input_pkt);
private:
    int ProcessPacketFromClient(Packet &input_pkt);
    int ProcessPacketFromBB1(Packet &input_pkt);
    int ProcessPacketFromBB2(Packet &input_pkt);
};

int main(int argc, char **argv) {

    char *opt_str = "f:m:h";
    string config_file, exe_mode;

    char c;
    while ((c = getopt(argc, argv, opt_str)) != -1) {
        switch (c) {
        case 'f':
            config_file = optarg;
            break;
        case 'm':
            exe_mode = optarg;
            break;
        case 'h':
        default:
            cerr << "Usage: ./test_bf -f configfile -m [debug|daemon] -h" << endl;
            break;
        }
    }
    if (config_file.empty || exe_mode.empty()) {
        cerr << "Parameter error:" << endl;
        cerr << "Usage: ./test_bf -f configfile -m [debug|daemon] -h" << endl;
        return -1;
    }

    TestBF *server = new TestBF;
    int ret = server->LoadConfig(true, config_file);
    if (ret < 0) {
        cerr << "Load config file error!" << endl;
        return -1;
    }

    server->Run();
    return 0;

}


int TestBF::ProcessPacket(Packet &input_pkt) {

    // 获得对内对外的ip和端口，用来分派数据包。（每个数据包到来都需要查找一次，效率不高，不过listen套接口一般很少）
    string outter_ip, inner_ip;
    uint16_t outter_port, inner_port;
    map<string, ListenSocketInfo>::iterator tmpit = listen_socket_map_.find("outter_data_channel");
    if (tmpit != listen_socket_map_.end()) {
        outter_ip = tmpit->second.ip_;
        outter_port = tmpit->second.port_;
    }

    tmpit = listen_socket_map_.find("inner_data_channel");
    if (tmpit != listen_socket_map_.end()) {
        inner_ip = tmpit->second.ip_;
        inner_port = tmpit->second.port_;
    }

    // 根据数据包的来源，分别进行处理。
    if (input_pkt.my_ipstr_ == outter_ip && input_pkt.my_port_ == outter_port) {
        // 这是一个从外网来的数据包（即从客户端来的数据包）

    }

}
