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

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <vector>
#include <map>
#include <list>

namespace NF {

class SocketAddr {
public:
    std::string ip_;
    uint16_t port_;
};

class Server {
public:
    // constructor/destructor
    Server() {};
    ~Server() {};

    int AddListenSocket(std::string &ip, uint16_t port) {
    };
    int AddUDPSocket(std::string &ip, uint16_t port) {
    };
private:
    std::vector<Socket> listen_socket_list_;
    std::vector<Socket> udp_socket_list_;

    std::list<Socket> server_socket_ready_list_;
    std::map<SocketAddr, std::list<Socket> > client_socket_list_;



};
}; // namespace NF

#endif // __SERVER_HPP__
