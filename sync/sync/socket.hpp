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

#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

#include <sys/socket.h>
#include <netinet/in.h>

#include <string>

namespace NF{


class Socket {

public:
    // constructors/destructors
    Socket(int sk) {
        sk_ = sk;
    };

    ~Socket() { };

    // setters/getters
    void set_sk(int sk) { sk_ = sk; };
    int sk() { return sk_; };

    void set_my_ipstr(std::string &ipstr) { my_ipstr_ = ipstr;};
    std::string& my_ipstr() { return my_ipstr_; };
    void set_my_ip(struct in_addr &ip) { my_ip_ = ip; };
    struct in_addr my_ip() { return my_ip_; };
    void set_my_port(uint16_t port) { my_port_ = port; };
    uint16_t my_port() { return my_port_; };

    void set_peer_ipstr(std::string &ipstr) { peer_ipstr_ = ipstr;};
    std::string& peer_ipstr() { return peer_ipstr_; };
    void set_peer_ip(struct in_addr &ip) { peer_ip_ = ip; };
    struct in_addr peer_ip() { return peer_ip_; };
    void set_peer_port(uint16_t port) { peer_port_ = port; };
    uint16_t peer_port() { return peer_port_; };

    // manipulators
    int Close() {
        close(sk_);
    };
    int GetError() {
        return 0;
    };
    int SetNonBlocking() {
        int flags = 0;
        flags = fcntl(sk_, F_GETFL, 0);
        if (flags == -1) {
            return -1;
        }
        if (fcntl(sk_, F_SETFL, flags | O_NONBLOCK) == -1) {
            return -1;
        }
        return 0;
    };
private:
    int sk_;

    std::string my_ipstr_;
    struct in_addr my_ip_;
    uint16_t my_port_;

    std::string peer_ipstr_;
    struct in_addr peer_ip_;
    uint16_t peer_port_;

private:
    // prohibits
    Socket(Socket&){};
    Socket& operator=(Socket&){};

};

};// namespace NF


#endif // __SOCKET_HPP__
