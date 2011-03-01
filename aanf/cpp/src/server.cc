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

int Server::LoadConfig(bool first_time, std::string &config_file) {

    if (first_time) {
        // 系统启动时读取配置文件
        Config cfg;
        try {
            cfg.readFile(config_file.c_str());
        } catch (FileIOException &e) {
            return -1;
        }


        map<string, Socket::SocketType> tmpmap1, tmpmap2;
        tmpmap1["tcp"] = Socket::T_TCP_LISTEN;
        tmpmap1["udp"] = Socket::T_UDP_SERVER;
        tmpmap2["bin"] = Socket::DF_BIN;
        tmpmap2["line"] = Socket::DF_LINE;
        tmpmap2["http"] = Socket::DF_HTTP;
        map<string, LogType> tmpmap3;
        tmpmap3["roll_by_day"] = T_ROLL_BY_DAY;
        tmpmap3["roll_by_size"] = T_ROLL_BY_SIZE;
        tmpmap3["roll_by_day_and_size"] = T_ROLL_BY_DAY_AND_SIZE;

        Setting st = cfg.lookup("server");

        my_connect_ipstr_ = st["my_connect_ip"];
        my_connect_port_ = st["my_connect_port"];
        // report
        report_server_ip_ = st["report_server_ip"];
        report_server_port_ = st["report_server_port"];
        report_interval_ = st["report_interval"];
        report_server_data_format_ = Socket::DF_BIN;
        report_server_type_ = Socket::T_TCP_CLIENT;
        // config
        config_server_ip_ = st["config_server_ip"];
        config_server_port_ = st["config_server_port"];
        config_check_interval_ = st["config_check_interval"];

        // logger
        log_server_ip_ = st["log_server_ip"];
        log_server_port_ = st["log_server_port"];
        log_server_data_format_ = Socket::DF_BIN;
        log_server_type_ = Socket::T_TCP_CLIENT;
        log_type_ = tmpmaap3[st["log_type"]];

        worker_num_ = st["worker_num_"];
        send_queue_num_ = st["send_queue_num"];

        Setting st2 = cfg.lookup("server.listen_socket");
        int tmp1 = st.getLength();


        for (int i = 0; i < tmp1; i++) {

            string name = st[i]("name");

            ListenSocketInfo tmpinfo;
            tmpinfo.ip_ = st[i]("ip");
            tmpinfo.port_ = st[i]("port");
            tmpinfo.data_format_ = st[i]("format");
            tmpinfo.type_ = st[i]("type");

            listen_socket_map_.insert(name, tmpinfo);
        }
        // 进行初始化工作
        // 初始化NetFrame
        int ret = 0;
        netframe_ = new NetFrame(send_queue_num_, worker_num_);
        // 初始化远程日志服务
        ret = InitRemoteLogger();
        if (ret < 0) {
            return -2
        }
        // 初始化远程数据上报服务
        ret = InitReporter();
        if (ret < 0) {
            return -2
        }
        // 初始化配置服务
        ret = InitConfigUpdater();
        if (ret < 0) {
            return -2
        }
        // 初始化侦听套接口
        ret = InitListenSocket();
        if (ret < 0) {
            return -2
        }
    } else {
        // 是动态更新配置文件
    }

    return 0;

}

void Server::LoadConfigSignalHandler(int signo, short events, CallBackArg *arg) {

        SignalCallBackArg *cb_arg = reinterpret_cast<SignalCallBackArg*>(arg);
        Server *srv = cb_arg->server_;
        int ret = srv->LoadConfig(false, srv->config_file_);

}

int Server::InitRemoteLogger() {

    Socket *sk = netframe_->socket_pool()->CreateClientSocket(log_server_ip_,
                                                              log_server_port_,
                                                              log_server_type_,
                                                              log_server_data_format_);
    if (sk == 0) {
        return -2;
    }
    netframe_->AddSocketToMonitor(sk);
    log_server_ready_ = true;
    return 0;
}

int Server::InitReporter() {

    Socket *sk = netframe_->socket_pool()->CreateClientSocket(report_server_ip_,
                                                              report_server_port_,
                                                              report_server_type_,
                                                              report_server_data_format_);
    if (sk == 0) {
        return -2;
    }
    netframe_->AddSocketToMonitor(sk);
    log_server_ready_ = true;
    return 0;
}

bool Server::IsConfigFileChanged() {
    return config_file_changed_;
}

int Server::SyncGetConfigFile() {

    MemBlock *send_buf;
    MemPool::GetMemPool()->GetMemBlock(1024, send_buf);

    ConfigUpdateRequest req;
    ConfigUpdateResponse rsp;

    // 构造请求
    req.Ver = 1001;
    // ...

    // 发送请求并接收应答
    MemBlock *recv_buf;
    ret = config_sr_->SyncSendRecv(send_buf, recv_buf);
    if (ret < 0) {
        MemPool::GetMemPool()->ReturnMemBlock(send_buf);
        //MemPool::GetMemPool()->ReturnMemBlock(recv_buf);
        return -2;
    }
    istream is;
    is.rdbuf()->pubsetbuf(recv_buf->start_, recv_buf->used_);
    rsp.ParseFromString(is);

    fstream fs;
    fs.open(server->config_file_, ios_base::in | ios_base::out | ios_base::binary);
    fs.write(rsp.ConfigFile.c_str(), rsp.ConfigFile.length());
    config_file_changed_ = true;
    return 0;
}

void Server::GetConfigThreadProc(ConfigUpdateThreadProcArg *arg) {

    Server *server = arg->server_;

    while (!server->cancel_) {
        int ret = SyncGetConfigFile();
        if (ret < 0) {
            return;
        }

        // sleep
        sleep(server->config_check_interval_);
    } // while
}

int Server::InitConfigUpdater() {

    int ret = SyncGetConfigFile();
    if (ret < 0) {
        return -2;
    }
    pthread_t id;
    ConfigUpdateThreadProcArg *arg = new ConfigUpdateThreadProcArg;
    arg->id_ = 0;
    arg->check_interval_ = config_check_interval_;
    arg->server_ = this;
    pthread_create(&id, NULL, GetConfigThreadProc, arg);
    config_server_ready_ = true;
    return 0;
}

int Server::InitListenSocket() {

    map<string, ListenSocketInfo>::iterator it, endit;
    it = listen_socket_map_.begin();
    endit = listen_socket_map_.end();

    int ret = 0;
    for (; it != endit; it++) {
        ret = netframe_->socket_pool()->CreateListenSocket(it->second.ip_,
                                                           it->second.port_,
                                                           it->second.type_,
                                                           it->second.data_format_);
        if (ret < 0) {
            return -1;
        }
    }
    return 0;
}

void Server::WorkderThreadProc(ThreadProcArg *arg) {

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
        // 先处理命令通道

        if (server->my_admin_ip_ == pkt->my_ipstr_ && server->my_admin_port_ == pkt->my_port_) {
            // 这是一个命令通道的数据
            ret = server->ProcessAdminPacket(pkt);
            if (ret < 0) {
                //
            }
        } else {
            // 这是普通数据包
            ret = server->ProcessPacket(pkt);
            if (ret < 0) {
                //
            }
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
        ThreadProcArg *arg = new ThreadProcArg
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

int Server::RegisterAdminCommand(std::string &cmd, AdminCmdFunc func) {

    if (!func) {
        return -1;
    }
    pair<map<string, AdminCmdFunc>::iterator, bool> ret =
        admin_cmd_map_.insert(pair<string, AdminCmdFunc>(cmd, func));
    if (ret.second) {
        return -2;
    }
    admin_cmd_map_[cmd] = func;
    return 0;
}

int Server::ProcessAdminCmdPacket(Packet &input_pkt) {

    if (input_pkt->data_format_ != Socket::DF_LINE) {
        return -1;
    }
    string cmd_options_lines(input_pkt->data_->start_, input_pkt->data_->start_ + input_pkt->data_->used_);

    size_t pos1, pos2;
    pos1 = pos2 = 0;
    while ((pos2 = cmd_options_lines.find_first_of(" \n", pos1)) != string::npos)  {
        string cmd_options_line = cmd_options_lines.substr(pos1, pos2);

        string cmd, options;
        // extract 'cmd'
        size_t pos = cmd_options_line.find_first_of(" \t");
        if (pos != string::npos) {
            cmd = cmd_options_line.substr(0, pos);
        }

        // extract 'options'
        pos = cmd_options_line.find_first_not_of("  \t", pos);

        if (pos != string::npos) {
            options = cmd_options_line.substr(pos);
        } else {
            options = "";
        }

        map<string, AdminCmdFunc>::iterator it = admin_cmd_map_.find(cmd);
        if (it != admin_cmd_map_.end()) {
            int ret = it->second(cmd, options); // 调用注册的admin函数
            if (ret < 0) {
                return -2;
            }
        }

        // 更新游标pos1/pos2
        if (pos2 +1 == cmd_options_lines.length()) {
            // 是最后一行了
            break;
        } else {
            // 否则
            pos1 = pos2 = pos2 + 1;
        }
    } // while




    return 0;
}

}; // namespace AANF
