 /*
    Copyright (C) <2011>  <ZHOU Xiaobo>

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

#ifndef _ZXB_PACKET_H_
#define _ZXB_PACKET_H_

class Packet {
public:
    Packet(struct timeval recv_time,
           std::string &peer_ipstr, uint16_t peer_port,
           std::string &my_ipstr, uint16_t my_port,
           Socket *from_socket);
public:
    struct timeval recv_time_;
    std::string peer_ipstr_;
    uint16_t peer_port_;
    std::string my_ipstr_;
    uint16_t my_port_;
    MemBlock *data_;
    int seq_;
private:
    Socket *from_socket_;
    // Prohibits
    Packet();
    Packet(Packet&);
    Packet& operator=(Packet&);

};
#endif
