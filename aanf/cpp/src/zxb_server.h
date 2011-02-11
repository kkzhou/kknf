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


#include "zxb_netframe.h"

namespace ZXB{

class Server {
public:
    // Server类是一个单体
    static Server* GetServerInstance();

    // 加载/重新加载配置文件，因为不是所有配置项都是可重新加载的，因此要区分。
    int LoadConfig(bool first_time, std::string &config_file);
    // 通过发送信号来触发配置文件的重新加载
    static void LoadConfigSignalHandler(int signo, short events, CallBackArg *arg);
    bool IsConfigFileChanged();

    // 初始化能写到远程日志服务器的logger，这个logger可写到远端，也可写在本地
    int InitLogger(std::string &to_ipstr, uint16_t to_port,
                   std::string &my_ipstr, uint16_t my_port);

    // 初始化能把数据上报到远端服务器的的reporter
    int InitReporter(std::string &to_ipstr, uint16_t to_port,
                   std::string &my_ipstr, uint16_t my_port);

    // 启动Server
    int Run();
private:
    Server(Netframe *netframe);
    ~Server();
    // 我用于侦听的ip和端口们
    std::vector<std::string> my_listen_ipstr_;
    std::vector<uint16_t> my_listen_port_;

    // 当我主动去连接其他服务器时，使用的ip和端口，一般来说用any就行了
    std::string my_connect_ipstr_;
    uint16_t my_connect_port_;

    int worker_num_;    // worker线程的数目

    // 核心类NetFrame的实例
    NetFrame *netframe_;
};

};// nameapce ZXB

