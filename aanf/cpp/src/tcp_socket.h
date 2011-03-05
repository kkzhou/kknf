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
#ifndef _TCP_SOCKET_H_
#define _TCP_SOCKET_H_

#include <string>

#include "socket.h"

namespace AANF {

class TcpSocket : public Socket {
public:
    virtual int PrepareListenSocket(std::string &listen_ip, uint16_t listen_port,
                           Socket::SocketType type,
                           Socket::DataFormat data_format);
    virtual int PrepareClientSocket(std::string &server_ip, uint16_t server_port,
                           Socket::SocketType type,
                           Socket::DataFormat data_format);
    virtual int PrepareServerSocket(int fd, Socket::SocketType type,
                           Socket::DataFormat data_format);
    virtual int Reconnect();
};

}; // namespace AANF
#endif
