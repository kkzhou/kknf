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


#ifndef __ASYNC_UDP_SOCKET_HPP__
#define __ASYNC_UDP_SOCKET_HPP__


namespace ZXBNF {

    class UDPSocket : public AsyncSocket {
    public:
	struct UDPMessage {
	    Buffer *data_;
	    Address addr_;
	};
    public:
	int BindOn(Address &addr);
	int OnReadable();
	int OnWritable();
    public:
	int SendMessage(Buffer *msg, int msg_len);
	int Messagize(Buffer **msg, int *msg_len) = 0;
	
    private:
	UDPMessage *msg_in_recv_;
	UDPMessage *send_msg_list_;
    };

};

#endif
