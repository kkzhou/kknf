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


#include "netframe.h"

namespace AANF {

class Server {
public:
    class SignalCallBackArg : public CallBackArg {
    public:
        Server *server_;
    };
    // 传递给线程函数的参数
    class WorkerThreadProcArg {
    public:
        Server *server_;
        int id_;
    };
    class ListenSocketInfo {
    public:
        std::string ip_;
        uint16_t port_;
        Socket::SocketType type_;
        Socket::DataFormat data_format_;
    };
public:
    // Server类是一个单体
    static Server* GetServerInstance();

    // 加载/重新加载配置文件，因为不是所有配置项都是可重新加载的，因此要区分。
    int LoadConfig(bool first_time, std::string &config_file);
    // 通过发送信号来触发配置文件的重新加载
    static void LoadConfigSignalHandler(int signo, short events, void *arg);
    bool IsConfigFileChanged();

    // 初始化能写到远程日志服务器的logger，这个logger可写到远端，也可写在本地，如果
    // 不进行初始化，则默认写在本地
    int InitRemoteLogger(std::string &to_ipstr, uint16_t to_port,
                   std::string &my_ipstr, uint16_t my_port,
                   uint32_t logid);

    // 初始化能把数据上报到远端服务器的的reporter
    int InitReporter(std::string &to_ipstr, uint16_t to_port,
                   std::string &my_ipstr, uint16_t my_port,
                   uint32_t reportid);

    // 启动Server
    int Run();

    // 实际处理业务逻辑的接口
    // 返回值：
    // 0: 成功
    // <0: 失败
    int ProcessPacket(Packet &input_pkt) = 0;
    // 线程函数，通过调用Server::ProcessPacket函数实现业务逻辑
    static void WorkerThreadProc(WorkerThreadProcArg *arg);

public:
    // 辅助接口
    int Log(bool remote, uint32_t level, const char *format, ...);
    int Report(std::string &property, int value);
private:
    Server();
    ~Server();
    // 我用于侦听的ip和端口们
    std::map<std::string, ListenSocketInfo> listen_socket_map_;

    // 当我主动去连接其他服务器时，使用的ip和端口，一般来说用any就行了
    std::string my_connect_ipstr_;
    uint16_t my_connect_port_;

    int worker_num_;    // worker线程的数目
    int send_queue_num_;

    // 配置文件相关（远程配置服务器，定期拉取配置文件，如果有变化则重新加载）
    std::string config_file_;
    std::string log_server_ip_;
    uint16_t log_server_port_;
    bool config_server_ready_;

    // 上报服务器
    std::string report_server_ip_;
    uint16_t report_server_port_;
    int report_interval_;
    Socket::SocketType report_server_type_;
    Socket::DataFormat report_server_data_format_;
    bool report_server_ready_;

    // 远程日志服务器
    std::string config_server_ip_;
    uint16_t config_server_port_;
    Socket::SocketType log_server_type_;
    Socket::DataFormat log_server_data_format_;
    int config_check_interval_;
    bool log_server_ready_;

    // 线程控制参数
    bool cancel_;
    // 核心类NetFrame的实例
    NetFrame *netframe_;
};

};// nameapce AANF

