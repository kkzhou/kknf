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

#ifndef ZXB_SOCKET_OPERATOR_H_
#define ZXB_SOCKET_OPERATOR_H_
namespace ZXB {

class Socket;

class SocketOperator {
public:
    enum SocketCmd {
        C_CLOSE = 1,
        C_RECONNECT,
        C_SHUTDOWN
    };
public:
    virtual int ReadHandler(Packet *&in_pack) = 0;
    virtual int WriteHandler() = 0;
    virtual int ErrorHandler(SocketCmd cmd);

public:
    SocketOperator();
    virtual ~SocketOperator();
    int set_socket(Socket *sk);
    Socket* socket();
}

private:
    Socket *socket_;
    // Prohibits
    SocketOperator(SocketOperator&);
    SocketOperator& operator=(SocketOperator&);
};



#endif
