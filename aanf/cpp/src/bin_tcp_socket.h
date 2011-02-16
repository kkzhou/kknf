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
#ifndef _BIN_TCP_SOCKET_H_
#define _BIN_TCP_SOCKET_H_

#include "tcp_socket.h"

namespace AANF {
class BinTcpSocket : public TcpSocket {
public:
    virtual int ReadHandler();
private:
    uint32_t len_field_; // BinTcpSocket采用LV格式，前4字节是长度域，此变量用于存储从套接口读出来的len域
    int len_field_read_;     // 长度域已读出几个字节。UGLY design
};

}; // namespace AANF
#endif
