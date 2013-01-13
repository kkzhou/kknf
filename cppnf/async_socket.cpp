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


#include "util.hpp"
#include "async_socket.hpp"

namespace ZXBNF {

    // AsyncTCPListenSocket
    AsyncTCPListenSocket::AsyncTCPListenSocket(Address &addr) {

	ENTERING;
	int ret = bind(socket(), (struct sockaddr*)&(addr.addr()), addr.addr_len());
	if (ret < 0) {
	    Invalidate();
	    LEAVING2;
	    return;
	}
	ret = listen(socket(), 1024);
	if (ret < 0) {
	    Invalidate();
	    LEAVING2;
	    return;
	}

	LEAVING;
	return;
    };
    int AsyncTCPListenSocket::OnAcceptable() {

	ENTERING;
	struct sockaddr_in new_addr;
	socklen_t addr_len;
	int newsk = accept(socket_, (struct sockaddr*)&new_addr, &addr_len);
	if (new_sk < 0) {
	    if (errno != EAGAIN || errno != EINTR) {
		LEAVING2;
		return -1;
	    }
	    LEAVING2;
	    return -2;
	}
	LEAVING;
	return new_sk;
    };

    // AsyncTCPLVListenSocket
    AsyncTCPDataSocket* AsyncTCPLVListenSocket::MakeNewSocket(int fd) {

	return new LVSocket(fd);
    };

    // AsyncTCPHTTPListenSocket
    AsyncTCPDataSocket* AsyncTCPHTTPListenSocket::MakeNewSocket(int fd) {

	return new HTTPSocket(fd);
    };

};
