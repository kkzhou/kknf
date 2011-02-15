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
#ifndef _LINE_TCP_SOCKET_H_
#define _LINE_TCP_SOCKET_H_

#include "tcp_socket.h"

namespace AANF {
class LineTcpSocket : public TcpSocket {
public:
    virtual int ReadHandler();
    virtual int WriteHandler();
    virtual ~LineTcpSocket();
    LineTcpSocket();
private:
    MemBlock *non_packetized_buf_;
};

}; // namespace AANF
#endif
