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

#include <netinet/in.h>
#include <map>
#include <string>

namespace AANF {
// 套接口池类，用于管理套接口，包括查找、创建、删除和清除空闲套接口等操作。
class SocketPool {
public:
    SocketPool();
    ~SocketPool();

    Socket* FindSocket(std::string &peer_ip, uint16_t peer_port, Socket::SocketType type);
    int DestroySocket(Socket *sk);
    Socket* CreateListenSocket(std::string &listen_ip, uint16_t listen_port,
                               Socket::SocketType type,
                               DataFormat data_format);

    Socket* CreateClientSocket(std::string &server_ip, uint16_t server_port,
                               Socket::SocketType type,
                               DataFormat data_format);

    Socket* CreateServerSocket(int fd,
                               Socket::SocketType type,
                               DataFormat data_format);

    int SweepIdleSocket(int max_idle_sec);

private:
    int AddSocket(Socket *sk);

    // 会出现竞争状态，因此要加锁
    std::map<std::string, Socket*> socket_map_;// Use string(IP:PORT:TYPE) as key
    pthread_mutex_t socket_map_lock_;
};

} // namespace AANF
#endif
