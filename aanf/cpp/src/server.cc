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


#include "server.h"
#include <libconfig.hh>

namespace AANF {

using namespace libconfig;

Server* Server::GetServerInstance() {
    static Server *inst = 0;
    if (inst) {
        return inst;
    }

    inst = new Server();
    return inst;
}

int Server::LoadConfig(bool first_time, std::string &config_file) {

    if (first_time) {
        // 系统启动时读取配置文件
        Config cfg;
        try {
            cfg.readFile(config_file.c_str());
        } catch (FileIOException &e) {
            return -1;
        }

        Setting st = cfg.lookup("server");

        my_connect_ipstr_ = st["my_connect_ip"];
        my_connect_port_ = st["my_connect_port"];
        report_server_ip_ = st["report_server_ip"];
        report_server_port_ = st["report_server_port"];
        report_interval_ = st["report_interval"];
        config_server_ip_ = st["config_server_ip"];
        config_server_port_ = st["config_server_port"];
        config_check_interval_ = st["config_check_interval"];
        worker_num_ = st["worker_num_"];
        send_queue_num_ = st["send_queue_num"];

        Setting st2 = cfg.lookup("server.listen_socket");
        int tmp1 = st.getLength();

        map<string, Socket::SocketType> tmpmap1, tmpmap2;
        tmpmap1["tcp"] = Socket::T_TCP_LISTEN;
        tmpmap1["udp"] = Socket::T_UDP_SERVER;
        tmpmap2["bin"] = Socket::DF_BIN;
        tmpmap2["line"] = Socket::DF_LINE;
        tmpmap2["http"] = Socket::DF_HTTP;

        for (int i = 0; i < tmp1; i++) {
            string name = st[i]("name");

            ListenSocketInfo tmpinfo;
            tmpinfo.ip_ = st[i]("ip");
            tmpinfo.port_ = st[i]("port");
            tmpinfo.data_format_ = st[i]("format");
            tmpinfo.type_ = st[i]("type");

            listen_socket_map_.insert(name, tmpinfo);
        }
        // 初始化NetFrame
        netframe_ = new NetFrame(send_queue_num_, worker_num_);

    } else {
        // 是动态更新配置文件
    }

    return 0;

}

void Server::LoadConfigSignalHandler(int signo, short events, CallBackArg *arg) ｛

        SignalCallBackArg *cb_arg = reinterpret_cast<SignalCallBackArg*>(arg);
        Server *srv = cb_arg->server_;
        int ret = srv->LoadConfig(false, srv->config_file_);

｝

int Server::InitRemoteLogger(std::string &to_ipstr, uint16_t to_port,
                       std::string &my_ipstr, uint16_t my_port) {

}


void Server::WorkderThreadProc(WorkerThreadProcArg *arg) {

    if (!arg) {
        return;
    }
    Server *server = arg->server_;

    while (!server->cancel_) {
        Packet *pkt = 0;
        int ret - server->netframe_->GetPacketFromRecvQueue(arg->id_, ptk);
        if (ret < 0) {
            break;
        }

        ret = server->ProcessPacket(pkt);
        if (ret < 0) {
            //
        }
    } // while
}

int Server::Run() {

    if (!netframe_) {
        return -1;
    }

    // 初始化监听套接口
    map<string, ListenSocketInfo>::interator it = listen_socket_map_.begin();
    map<string, ListenSocketInfo>::interator endit = listen_socket_map_.end();
    for (; it != endit; it++) {
        netframe_->socket_pool()->CreateListenSocket(it->second.ip_,
                                                     it->second.port_,
                                                     it->second.type_,
                                                     it->second.data_format_)
    }

    // 初始化RemoteLogger，如果有的话
    if (!(log_server_ip_.empty() || log_server_port_ == 0)) {

        netframe_->socket_pool()->CreateClientSocket(log_server_ip_, log_server_port_,
                                                     log_server_type_, log_server_data_format_);
    }
    // 初始化Reporter，如果有的话
        if (!(report_server_ip_.empty() || report_server_port_ == 0)) {

        netframe_->socket_pool()->CreateClientSocket(report_server_ip_, report_server_port_,
                                                     report_server_type_, report_server_data_format_);
    }

    // 初始化Worker线程
    pthread_t tid[worker_num_];
    for (int i = 0; i < worker_num_; i++) {
        WorkerThreadProcArg *arg = new WorkerThreadProcArg
        arg->server_ = this;
        arg->id_ = i;
        pthread_create(&tid[i], NULL, Server::WorkerThreadProc, arg);
    }
    // 等待线程结束
    for (int i = 0; i < worker_num_; i++) {
        pthread_join(tid[i], NULL);
    }
    return 0;
}

}; // namespace AANF
