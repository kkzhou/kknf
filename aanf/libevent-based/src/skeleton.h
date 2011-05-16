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
        Skeleton *skeleton_;
    };
    // 传递给线程函数的参数
    class ThreadProcArg {
    public:
        Skeleton *skeleton_;
        int id_;
    };

    class ListenSocketInfo {
    public:
        std::string ip_;
        uint16_t port_;
        Socket::SocketType type_;
        Socket::DataFormat data_format_;
    };

    typedef (int)(*AdminCmdFunc)(std::string&);

public:
    Skeleton();
    ~Skeleton();

    // 初始化所有侦听套接口
    int InitListenSocket();

    // 初始化skeleton
    int Init();

    // 启动
    int Run();

    // 处理命令通道数据包的接口
    // 返回值：
    // 0: 成功
    // <0: 失败
    int ProcessAdminCmdMessage(Message &input_pkt);
    int RegisterAdminCmd(std::string &cmd, AdminCmdFunc func);
    // 实际处理业务逻辑的接口，如果是Line格式，则input_pkt里装的可能是多个报文，
    // 其它格式都是一个报文。UGLY design
    // 返回值：
    // 0: 成功
    // <0: 失败
    int ProcessMessage(Message &input_pkt) = 0;
    // 线程函数，通过调用Skeleton::ProcessMessage函数实现业务逻辑
    static void WorkerThreadProc(ThreadProcArg *arg);

private:
    // 我用于侦听的ip和端口们
    std::map<std::string, ListenSocketInfo> listen_socket_map_;
    string my_admin_ip_;    // 这两个变量是包含在上面的map里的，这里
    uint16_t my_admin_port_;// 重复存储是为了避免每次都查找map
    bool listen_socket_ready_;

    // 当我主动去连接其他服务器时，使用的ip和端口，一般来说用any就行了
    std::string my_connect_ipstr_;
    uint16_t my_connect_port_;

    int worker_num_;    // worker线程的数目
    int send_queue_num_;

    // 配置文件相关
    std::string config_file_;

    // 远程日志服务器
    std::string log_file_;
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

