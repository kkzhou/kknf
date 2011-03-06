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

class Skeleton {
public:
    class SignalCallBackArg : public CallBackArg {
    public:
        Skeleton *skeletion_;
    };
    // 传递给线程函数的参数
    class ThreadProcArg {
    public:
        Skeleton *skeleton_;
        int id_;
    };
    class ConfigUpdateThreadProcArg : public ThreadProcArg {
    public:
        int check_interval_;
    };
    class ListenSocketInfo {
    public:
        std::string ip_;
        uint16_t port_;
        Socket::SocketType type_;
        Socket::DataFormat data_format_;
    };
    enum LogType {
        T_ROLL_BY_SIZE = 1,
        T_ROLL_BY_DAY,
        T_ROLL_BY_DAY_AND_SIZE
    };

    typedef (int)(*AdminCmdFunc)(std::string&);

public:
    Skeleton();
    ~Skeleton();
    // 线程函数，用来获取最新的配置文件
    static void GetConfigThreadProc(ThreadProcArg *arg);
    int SyncGetConfigFile();
    // 加载/重新加载配置文件，因为不是所有配置项都是可重新加载的，因此要区分。
    // 返回值：
    // 0: 成功
    // -1: 参数错误
    // <-1: 其他错误
    int LoadConfig(bool first_time, std::string &config_file);
    // 通过发送信号来触发配置文件的重新加载
    static void LoadConfigSignalHandler(int signo, short events, void *arg);
    // 检查配置文件是否发生变化，只是检查本地配置文件，
    // 至于如何从远程拉取配置文件，是另外的进程的任务。
    bool IsConfigFileChanged();

    // 初始化能写到远程日志服务器的logger，这个logger可写到远端，也可写在本地，如果
    // 不进行初始化，则默认写在本地
    // 返回值:
    // 0: 成功
    // -1: 参数错误
    // <-1: 其他错误
    int InitRemoteLogger();

    // 初始化能把数据上报到远端服务器的的reporter
    // 0: 成功
    // -1: 参数错误
    // <-1: 其他错误
    int InitReporter();
    // 初始化定期更新配置文件的服务，需要用独立线程去实现。
    // 0: 成功
    // -1: 参数错误
    // <-1: 其他错误
    int InitConfigUpdater();

    // 初始化所有侦听套接口
    int InitListenSocket();

    // 启动
    int Run();

    // 处理命令通道数据包的接口
    // 返回值：
    // 0: 成功
    // <0: 失败
    int ProcessAdminCmdPacket(Packet &input_pkt);
    int RegisterAdminCmd(std::string &cmd, AdminCmdFunc func);
    // 实际处理业务逻辑的接口，如果是Line格式，则input_pkt里装的可能是多个报文，
    // 其它格式都是一个报文。UGLY design
    // 返回值：
    // 0: 成功
    // <0: 失败
    int ProcessPacket(Packet &input_pkt) = 0;
    // 线程函数，通过调用Skeleton::ProcessPacket函数实现业务逻辑
    static void WorkerThreadProc(ThreadProcArg *arg);

public:
    // 辅助接口
    int Log(bool remote, uint32_t logid, uint32_t level, const char *format, ...);
    int Report(uint32_t report_id, std::string &property, int value);
private:
    // 我用于侦听的ip和端口们
    std::map<std::string, ListenSocketInfo> listen_socket_map_;
    string my_admin_ip_;    // 这两个变量是包含在上面的map里的，这里
    uint16_t my_admin_port_;// 重复存储是为了避免每次都查找map

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
    BinSyncSR *config_sr_;
    bool config_file_changed_;

    // 上报服务器
    std::string report_server_ip_;
    uint16_t report_server_port_;
    int report_interval_;
    Socket::SocketType report_server_type_;
    Socket::DataFormat report_server_data_format_;
    bool report_server_ready_;
    uint32_t report_id_;

    // 远程日志服务器
    std::string log_file_;
    std::string config_server_ip_;
    uint16_t config_server_port_;
    Socket::SocketType log_server_type_;
    Socket::DataFormat log_server_data_format_;
    int config_check_interval_;
    bool log_server_ready_;
    uint32_t log_id_;
    LogType log_type_;
    uint32_t log_level_;

    // 线程控制参数
    bool cancel_;

    // 命令<->函数 映射map
    std::map<std::string, AdminCmdFunc> admin_cmd_map_;

    // 核心类NetFrame的实例
    NetFrame *netframe_;
};

};// nameapce AANF

