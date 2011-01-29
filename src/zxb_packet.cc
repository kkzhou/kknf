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

#include "zxb_packet.h"

Packet::Packet(struct timeval recv_time,
               std::string &peer_ipstr, uint16_t peer_port,
               std::string &my_ipstr, uint16_t my_port,
               MemBlock *data, Socket *from_socket) {

    recv_time_ = recv_time;
    peer_ipstr_ = peer_ipstr;
    peer_port_ = peer_port;
    my_ipstr_ = my_ipstr;
    my_port_ = my_port;
    from_socket_ = from_socket;
}

~Packet() {
}
