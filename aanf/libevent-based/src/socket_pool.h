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

#ifndef _SOCKET_POOL_H_
#define _SOCKET_POOL_H_

#include <map>
#include <string>
#include <socket.h>

namespace AANF {
// 套接口池类，用于管理套接口，包括查找、创建、删除和清除空闲套接口等操作。
// 所有套接口被维护在一个map里，map的key是ip:port:type。
// 如果是listen套接口，则用myip:myport:type
// 如果是server套接口，则用peerip:peerport:type
// 如果是client套接口，则用peerip:peerport:type(可能一对多)
class SocketPool {
public:
    class SocketKey {
    public:
        std::string ip_;
        uint16_t port_;
        Socket::SocketType type_;
    };

    SocketPool();
    ~SocketPool();
    // 查找Socket对象
    Socket* FindSocket(SocketKey &key);

    // 销毁一个Socket对象，包括系统资源的回收，以及缓存的释放
    int DestroySocket(Socket *sk);

    // 创建一个侦听套接口，包括异步socket/bind/listen
    Socket* CreateListenSocket(std::string &listen_ip, uint16_t listen_port,
                               Socket::SocketType type,
                               Socket::DataFormat data_format);

    // 创建一个client套接口，包括socket/connect
    Socket* CreateClientSocket(std::string &server_ip, uint16_t server_port,
                               Socket::SocketType type,
                               DataFormat data_format);

    // 创建一个server套接口，即根据accept函数返回的新socket，获取响应Socket对象所需的信息
    Socket* CreateServerSocket(int fd,
                               Socket::SocketType type,
                               DataFormat data_format);

    // 清除闲置套接口
    int SweepIdleSocket(int max_idle_sec);

private:
    // 把套接口加入到map中
    int AddSocket(Socket *sk);

    // 用于记录所有套接口。
    std::map<SocketKey, std::list<Socket*>> socket_map_;

    // 会出现竞争状态，因此要加锁
    pthread_mutex_t socket_map_lock_;
};

} // namespace AANF
#endif
