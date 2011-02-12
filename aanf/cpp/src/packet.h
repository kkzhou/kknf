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

#ifndef _PACKET_H_
#define _PACKET_H_

#include "socket.h"
#include "memblock.h"

namespace AANF {

class MemBlock;

class Packet {
public:
    Packet(struct timeval &create_time,
           std::string &peer_ipstr, uint16_t peer_port,
           std::string &my_ipstr, uint16_t my_port,
           Socket::SocketType type, Socket::DataFormat df,
           MemBlock *data) {
                create_time_ = create_time;
                peer_ipstr_ = peer_ipstr;
                peer_port_ = peer_port;
                my_ipstr_ = my_ipstr;
                my_port_ = my_port;
                data_format_ = fd;
                type_ = type;
                data_ = data;
           };
    ~Packet();

public:
    struct timeval create_time_;
    std::string peer_ipstr_;
    uint16_t peer_port_;
    std::string my_ipstr_;
    uint16_t my_port_;
    Socket::SocketType sk_type_;
    Socket::DataFormat data_format_;
    MemBlock *data_;

private:
    // Prohibits
    Packet();
    Packet(Packet&);
    Packet& operator=(Packet&);

};

};// namespace AANF
#endif