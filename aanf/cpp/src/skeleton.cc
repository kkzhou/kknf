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
#include <iostream>
#include <libconfig.hh>
#include "server.h"
#include "utils.h"
#include "UpdateConfig.pb.h"

namespace AANF {

using namespace libconfig;
using namespace std;
using namespace Protocol;

int Skeleton::Init() {

    // 进行初始化工作
    // 初始化NetFrame
    int ret = 0;
    netframe_ = new NetFrame(send_queue_num_, worker_num_);

    // 初始化日志服务
    ret = SLog::InitSLog(log_level_, log_type_);
    if (ret < 0) {
        cerr << "SLog init error!" << endl;
        return -2;
    }
    SLOG(LogLevel.L_INFO, "SLog is ready\n");
    ret = InitRemoteLogger();
    if (ret < 0) {
        SLOG(LogLevel.L_INFO, "InitRemoteLogger() error!\n");
    } else {
        log_server_ready_ = true;
    }

    // 初始化远程数据上报服务
    ret = InitReporter();
    if (ret < 0) {
        SLOG(LogLevel.L_INFO, "InitReporter() error!\n");
    } else {
        report_server_ready_ = true;
    }
    // 初始化配置服务
    ret = InitConfigUpdater();
    if (ret < 0) {
        SLOG(LogLevel.L_INFO, "InitConfigUpdater() error!\n");
    } else {
        config_server_ready_ = true;
    }

    // 初始化侦听套接口
    ret = InitListenSocket();
    if (ret < 0) {
        SLOG(LogLevel.L_INFO, "InitListenSocket() error!\n");
    } else {
        listen_socket_ready_ = true;
    }
    LEAVING;
    return 0;
}

int Skeleton::LoadConfig(bool first_time, std::string &config_file) {

    if (first_time) {
        // 系统启动时读取配置文件
        Config cfg;
        try {
            cfg.readFile(config_file.c_str());
        } catch (FileIOException &e) {
            cerr << "cant read config file: " << config_file << endl;
            return -1;
        }

        // 建立一些map，用来把配置文件中的字符串映射成程序中需要的类型
        map<string, Socket::SocketType> tmpmap1;
        tmpmap1["tcp"] = Socket::SocketType.T_TCP_LISTEN;
        tmpmap1["udp"] = Socket::SocketType.T_UDP_SERVER;

        map<string, Socket::DataFormat> tmpmap2;
        tmpmap2["bin"] = Socket::DataFormat.DF_BIN;
        tmpmap2["line"] = Socket::DataFormat.DF_LINE;
        tmpmap2["http"] = Socket::DataFormat.DF_HTTP;

        map<string, LogType> tmpmap3;
        tmpmap3["roll_by_day"] = LogType.T_ROLL_BY_DAY;
        tmpmap3["roll_by_size"] = LogType.T_ROLL_BY_SIZE;
        tmpmap3["roll_by_day_and_size"] = LogType.T_ROLL_BY_DAY_AND_SIZE;

        map<string, uint32_t> tmpmap4;
        tmpmap4["fatal"] = LogLevel.L_FATAL;
        tmpmap4["debug"] = LogLevel.L_DEBUG;
        tmpmap4["syserr"] = LogLevel.L_SYSERR;
        tmpmap4["logicerr"] = LogLevel.L_LOGICERR;
        tmpmap4["info"] = LogLevel.L_INFO;
        tmpmap4["func"] = LogLevel.L_FUNC;

        Setting st = cfg.lookup("skeleton");

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
        log_file_ = st["log_file"];

        string tmp_log_level_str = st["log_level"];
        uint32_t tmp_log_level = 0;
        map<string, LogLevel>::iterator it, endit;
        it = tmpmap4.begin();
        endit = tmpmap4.end();
        for (; it != endit; it++) {
            if (tmp_log_level_str.find(it->first) != string::npos) {
                tmp_log_level |= it->second;
            }
        }

        worker_num_ = st["worker_num_"];
        send_queue_num_ = st["send_queue_num"];

        Setting st2 = cfg.lookup("skeleton.listen_socket");
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

    } else {
        // 是动态更新配置文件
    }
    LEAVING;
    return 0;

}

void Skeleton::LoadConfigSignalHandler(int signo, short events, CallBackArg *arg) {

    ENTERING
    SignalCallBackArg *cb_arg = reinterpret_cast<SignalCallBackArg*>(arg);
    Skeleton *skeleton = cb_arg->skeleton_;
    int ret = skeleton->LoadConfig(false, srv->config_file_);
    if (ret < 0) {
        SLOG(LogLevel.L_SYSERR, "LoadConfig() error: %d\n", ret);
    }
    LEAVING;
    return;
}

int Skeleton::InitRemoteLogger() {

    ENTERING;
    Socket *sk = netframe_->socket_pool()->CreateClientSocket(log_server_ip_,
                                                              log_server_port_,
                                                              log_server_type_,
                                                              log_server_data_format_);
    if (sk == 0) {
        SLOG(LogLevel.L_SYSERR, "CreateClientSocket() for remote log error!\n");
        LEAVING;
        return -2;
    }
    netframe_->AddSocketToMonitor(sk);
    log_server_ready_ = true;
    LEAVING;
    return 0;
}

int Skeleton::InitReporter() {

    ENTERING;
    Socket *sk = netframe_->socket_pool()->CreateClientSocket(report_server_ip_,
                                                              report_server_port_,
                                                              report_server_type_,
                                                              report_server_data_format_);
    if (sk == 0) {
        SLOG(LogLevel.L_SYSERR, "CreateClientSocket() for reporter error!\n");
        LEAVING;
        return -2;
    }

    netframe_->AddSocketToMonitor(sk);
    log_server_ready_ = true;
    LEAVING;
    return 0;
}

bool Skeleton::IsConfigFileChanged() {
    ENTERING;
    LEAVING;
    return config_file_changed_;
}

int Skeleton::SyncGetConfigFile() {

    MemBlock *send_buf;
    MemPool::GetMemPool()->GetMemBlock(1024, send_buf);

    PacketFormat req, rsp;

    // 构造请求
    req.set_version(1000);
    req.set_service_id(11);
    req.set_type(111);
    req.set_md5("aaa");
    req.set_length(req.ByteSize());

    MemBlock *send_buf;
    int ret = 0;
    ret = MemPool::GetMemPool()->GetMemBlock(req.length());
    if (ret < 0) {
        SLOG(LogLevel.L_SYSERR, "GetMemBlock() error: %d\n", ret);
        LEAVING;
    }
    req.SerializeToArray(send_buf.start_, req.length());

    // 发送请求并接收应答
    MemBlock *recv_buf;
    ret = config_sr_->SyncSendRecv(send_buf, recv_buf);
    if (ret < 0) {
        MemPool::GetMemPool()->ReturnMemBlock(send_buf);
        MemPool::GetMemPool()->ReturnMemBlock(recv_buf);
        SLOG(LogLevel.L_SYSERR, "SyncSendRecv() error: %d\n", ret);
        LEAVING;
        return -2;
    }

    istream is;
    is.rdbuf()->pubsetbuf(recv_buf->start_, recv_buf->used_);
    rsp.ParseFromString(is);

    fstream fs;
    fs.open(server->config_file_, ios_base::in | ios_base::out | ios_base::binary);
    fs.write(rsp.ConfigFile.c_str(), rsp.ConfigFile.length());
    config_file_changed_ = true;
    MemPool::GetMemPool()->ReturnMemBlock(send_buf);
    MemPool::GetMemPool()->ReturnMemBlock(recv_buf);
    LEAVING;

    return 0;
}

void Skeleton::GetConfigThreadProc(ConfigUpdateThreadProcArg *arg) {

    ENTERING;
    Skeleton *skeleton = arg->skeleton_;

    while (!skeleton->cancel_) {
        int ret = SyncGetConfigFile();
        if (ret < 0) {
            SLOG(LogLevel.L_SYSERR, "SyncGetConfigfile() error: %d\n", ret);
        }

        if (config_file_changed_) {
            pthread_(NetFrame::SignalNo.SN_USR_1);
        }
        // sleep
        sleep(skeleton->config_check_interval_);
    } // while
    LEAVING;
}

int Server::InitConfigUpdater() {

    ENTERING;
    int ret = SyncGetConfigFile();
    if (ret < 0) {
        return -2;
    }
    pthread_t id;
    ConfigUpdateThreadProcArg *arg = new ConfigUpdateThreadProcArg;
    arg->id_ = 0;
    arg->check_interval_ = config_check_interval_;
    arg->skeleton_ = this;
    pthread_create(&id, NULL, GetConfigThreadProc, arg);
    config_server_ready_ = true;
    LEAVING;
    return 0;
}

int Server::InitListenSocket() {

    ENTERING;
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

            SLOG(LogLevel.L_SYSERR, "CreateListenSocket() error: %d\n", ret);
            LEAVING;
            return -1;
        }
    }
    LEAVING;
    return 0;
}

void Skeleton::WorkderThreadProc(ThreadProcArg *arg) {

    ENTERING;
    if (!arg) {
        SLOG(LogLevel.L_LOGICERR, "parameter invalid\n");
        LEAVING;
        return;
    }
    Skeleton *skeleton = arg->skeleton_;

    while (!skeleton->cancel_) {
        Packet *pkt = 0;
        int ret - skeleton->netframe_->GetPacketFromRecvQueue(arg->id_, ptk);
        if (ret < 0) {
            break;
        }
        // 先处理命令通道

        if (skeleton->my_admin_ip_ == pkt->my_ipstr_ && skeleton->my_admin_port_ == pkt->my_port_) {
            // 这是一个命令通道的数据
            ret = skeleton->ProcessAdminPacket(pkt);
            if (ret < 0) {
                //
            }
        } else {
            // 这是普通数据包
            ret = skeleton->ProcessPacket(pkt);
            if (ret < 0) {
                //
            }
        }
    } // while
    LEAVING;
}

int Server::Run() {

    ENTERING;
    if (!netframe_) {
        SLOG(LogLevel.L_LOGICERR, "parameter invalid\n");
        LEAVING;
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
        arg->skeleton_ = this;
        arg->id_ = i;
        pthread_create(&tid[i], NULL, Server::WorkerThreadProc, arg);
    }
    // 等待线程结束
    for (int i = 0; i < worker_num_; i++) {
        pthread_join(tid[i], NULL);
    }
    LEAVING;
    return 0;
}

int Skeleton::RegisterAdminCommand(std::string &cmd, AdminCmdFunc func) {

    ENTERING;
    if (!func) {
        SLOG(LogLevel.L_LOGICERR, "parameter invalid\n");
        LEAVING;
        return -1;
    }
    map<string, AdminCmdFunc>::iterator it =
        admin_cmd_map_.find(cmd, func);
    if (it != admin_cmd_map_.end()) {
        SLOG(LogLevel.L_LOGICERR, "the admin command has alreay registered\n");
        LEAVING;
        return -2;
    }
    admin_cmd_map_[cmd] = func;
    LEAVING;
    return 0;
}

int Skeleton::ProcessAdminCmdPacket(Packet &input_pkt) {

    ENTERING;
    if (input_pkt->data_format_ != Socket::DF_LINE) {
        SLOG(LogLevel.L_LOGICERR, "parameter invalid\n");
        LEAVING;
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
                SLOG(LogLevel.L_LOGICERR, "admin command executed error\n");
                LEAVING;
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
    LEAVING;
    return 0;
}

}; // namespace AANF
