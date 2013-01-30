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

#ifndef __ASYNC_LV_SOCKET_HPP__
#define __ASYNC_LV_SOCKET_HPP__


#include "async_tcp_socket.hpp"

namespace ZXBNF {

    class LVListenSocket : public AsyncTCPListenSocket {
    public:
	LVListenSocket() {};
	~TCPLVListenSocket() {};

	virtual AsyncTCPDataSocket* MakeNewSocket(int fd) {
	    LVSocket *sk = new LVSocket;
	    sk->set_fd(fd);
	    return sk;
	};
    };

    class LVSocket : public AsyncTCPDataSocket {
    public:
	virtual int MessageSize() {

	    if (!msg_in_recv_) {
		return 0;
	    }
	    if (msg_in_recv_->data->tail - msg_in_recv_->data->head >= 4) {
		msg_in_recv_size_ = htonl(*(int*)(msg_in_recv_->data->start + msg_in_recv_->data->head));
		return msg_in_recv_size_;
	    }
	    return 0;
	};
    };
};

#enidf
