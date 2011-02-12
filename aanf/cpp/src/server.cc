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


#include "zxb_server.h"
#include <libconfig.hh>

namespace ZXB{

using namespace libconfig;

Server* Server::GetServerInstance() {
    static Server *inst = 0;
    if (inst) {
        return inst;
    }

    NetFrame netframe = new NetFrmae();
    inst = new Server(netframe);
    return inst;
}

int Server::LoadConfig(bool first_time, std::string &config_file) {

    Config cfg;
    try {
        cfg.readFile(config_file.c_str());
    } catch (FileIOException &e) {
        return -1;
    }

    Setting st = cfg.lookup("server");
    int tmp1 = st.getLength();

    for (int i = 0; i < tmp1; i++) {

        my_listen_ipstr_.push_back(st("my_listen_ip")[i]);
        my_listen_port_.push_back(st("my_listen_port")[i]);
    }

    my_connect_ipstr_ = st["my_connect_ip"];
    my_connect_port_ = st["my_connect_port"];
    report_server_ip_ = st["report_server_ip"];
    report_server_port_ = st["report_server_port"];
    report_interval_ = st["report_interval"];
    config_server_ip_ = st["config_server_ip"];
    config_server_port_ = st["config_server_port"];
    config_check_interval_ = st["config_check_interval"];

}

void Server::LoadConfigSignalHandler(int signo, short events, CallBackArg *arg) ｛

        SignalCallBackArg *cb_arg = reinterpret_cast<SignalCallBackArg*>(arg);
        Server *srv = cb_arg->server_;
        int ret = srv->LoadConfig(false, srv->config_file_);

｝

int Server::InitLogger(std::string &to_ipstr, uint16_t to_port,
                       std::string &my_ipstr, uint16_t my_port) {

}
}; // namespace ZXB
