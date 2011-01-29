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
class MemBlock;

class SocketOperator {
public:
    enum SocketCmd {
        C_CLOSE = 1,
        C_RECONNECT,
        C_SHUTDOWN
    };
public:
    int ReadHandler(Packet *&in_pack);
    virtual int BinReadHandler(Packet *&in_pack);
    virtual int HttpReadHandler(Packet *&in_pack);
    virtual int LineReadHandler(Packet *&in_pack);
    virtual int OtherReadHandler(Packet *&in_pack);

    virtual int WriteHandler();
    virtual int AcceptHandler();
    virtual int AcceptHandler();
    virtual int ErrorHandler(enum SocketCmd cmd);

public:
    SocketOperator();
    ~SocketOperator();
    int set_socket(Socket *sk);
    Socket* socket();

    static int GetSocketError(int fd, int &error);

    int PrepareListenSocket(std::string &my_ipstr, uint16_t my_port,
                            enum SocketType type);

    // When processing a request usually spawns several requests to the
    // server(s) behind this server.
    static int AsyncSend(std::string &to_ipstr, uint16_t to_port,
                              std::string &my_ipstr, uint16_t my_port, int &seq,
                              MemBlock *data, enum SocketType type, SocketOperator *&sk_used);

    // Actually only UDP use AyncRecv just like TCP's listen
    int AsyncRecv(std::string &my_ipstr, uint16_t my_port,
                  enum SocketType type, SocketOperator *&sk_used);
private:
    Socket *socket_;
    // Prohibits
    SocketOperator(SocketOperator&);
    SocketOperator& operator=(SocketOperator&);
};

};

#endif
