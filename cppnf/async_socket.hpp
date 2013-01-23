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


#ifndef __ASYNC_SOCKET_HPP__
#define __ASYNC_SOCKET_HPP__

#include "buffer.hpp"

namespace ZXBNF {

    // basic socket
    class AsyncSocket {
    protected:
	// constroctors & destructors
	AsyncSocket() :socket_(-1), error_(false){};
	virtual ~AsyncSocket() { close(socket_); };
    public:
	void Invalidate() { error_ = true; };
	// getters & settters
	inline int socket() { return socket_; };
	inline void set_socket(int socket) { socket_ = socket; };
	inline bool error() { return error_; };
	
    private:
	bool error_;
	int socket_;
    private:
	// prohibits
	AsyncSocket(AsyncSocket&){};
	AsyncSocket& operator=(AsyncSocket&){};

    };
};

#endif
