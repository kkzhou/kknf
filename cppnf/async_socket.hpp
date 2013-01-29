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
    public:
	enum State {
	    S_OK = 1,
	    S_TO_CLOSE,
	    S_TO_CLOSE_AFTER_SEND_ALL_DATA
	};
    protected:
	// constroctors & destructors
	AsyncSocket() :state_(S_OK), fd_(-1), mempool_(0){};
	virtual ~AsyncSocket() { close(fd_); };
    public:
	// getters & settters
	inline int fd() { return fd_; };
	inline void set_fd(int fd) { fd_ = fd; };
	inline State state() { return state_; };
	inline void set_state(State s) { state_ = s; };
	inline MemPool* mempool() { return mempool_; };
	inline void set_mempool(MemPool *pool) { mempool_ = pool; };
    public:
	inline int Nonblock() {
	    int flags; 
	    if ((flags = fcntl(fd(), F_GETFL, 0)) < 0) { 
		return -1;
	    } 
	    if (fcntl(fd(), F_SETFL, flags | O_NONBLOCK) < 0) { 
		return -2;
	    }
	    return 0;
	};
	
    private:
        State state_;
	int fd_;
	MemPool *mempool_;
    private:
	// prohibits
	AsyncSocket(AsyncSocket&){};
	AsyncSocket& operator=(AsyncSocket&){};

    };
};

#endif
