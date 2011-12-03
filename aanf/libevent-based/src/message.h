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

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "socket.h"

namespace AANF {

class MemBlock;

// Message类只是用来存储一些信息，没有操作函数。
//
class Message {
public:
    Message(struct timeval &create_time,
           std::string &peer_ipstr, uint16_t peer_port,
           std::string &my_ipstr, uint16_t my_port,
           Socket::SocketType type, Socket::DataFormat df,
           MemBlock *data) {
        ENTERING;
        create_time_ = create_time;
        peer_ipstr_ = peer_ipstr;
        peer_port_ = peer_port;
        my_ipstr_ = my_ipstr;
        my_port_ = my_port;
        data_format_ = fd;
        type_ = type;
        data_ = data;
        LEAVING;
        };
    ~Message() {
        ENTERING;
        if (data_) {
            SLOG(LogLevel::L_DEBUG, "this Message contains data");
            MemPool::GetMemPool()->ReturnMemBlock(data_);
        }

        LEAVING;
    };

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
    Message();
    Message(Message&);
    Message& operator=(Message&);

};

};// namespace AANF
#endif
