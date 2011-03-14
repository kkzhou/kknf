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
#include "ProtoForTest.pb.h"

using std;
using namespace AANF;
using namespace Protocol;

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

    TestBF *bf = new TestBF;
    int ret = bf->LoadConfig(true, config_file);
    if (ret < 0) {
        cerr << "Load config file error!" << endl;
        return -1;
    }
    bf->Init();
    bf->Run();
    return 0;

}




int TestBF::ProcessPacket(Packet &input_pkt) {

    PacketFormat req;
    req.ParseFromArray(input_pkt.data_->start_, input_pkt.data_->used_);
    if (req.type() == 1) {
        SLOG(LogLevel.L_LOGICERR, "input packet is type=%d\n", req.type());
        PacketFormat bb1req;
        GetUserIDByKeywordRequest get_userid_by_keyword_req;
        get_userid_by_keyword_req.set_user_name_keyword(req.cl);
    }


}
