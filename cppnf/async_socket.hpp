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
	AsyncSocket() :Event(-1), error_(false){};
	virtual ~AsncSocket() { close(socket_); };
    private:
	// prohibits
	AsyncSocket(AsyncSocket&){};
	AsyncSocket& operator=(AsyncSocket&){};
    public:
	void Invalidate() { error_ = true; };
	// getters & settters
	inline int socket() { return socket_; };
	inline void set_socket(int socket) { socket_ = socket; };
	inline bool error() { return error_; };
	
    private:
	bool error_;
	int socket_;
    };

    class AsyncTCPSocket : public AsyncSocket {
    protected:
	// constructors & destructors
	AsyncTCPSocket() {};
    private:
	// prohibits
	AsyncTCPSocket(AsyncTCPSOcket&){};
	AsyncTCPSocket& operator=(AsyncTCPSocket&){};
    };

    class AsyncTCPListenSocket : public AsyncTCPSocket {
    protected:
	AsyncTCPListenSocket() {};
	AsyncTCPListenSocket(Address &addr);

	int OnAcceptable();
	virtual int OnNewSocket(AsyncTCPDataSocket** sk) = 0;
    private:
    };

    class AsyncTCPLVListenSocket : public AsyncTCPListenSocket {
    public:
	AsyncTCPLVListenSocket(Address &addr) : AsyncTCPListenSocket(addr) {};
	~AsyncTCPLVListenSocket() {};

	virtual AsyncTCPDataSocket* MakeNewSocket(int fd);
    };

    class AsyncTCPHTTPListenSocket : public AsyncTCPListenSocket {
    public:
	AsyncTCPHTPListenSocket(Address &addr) : AsyncTCPListenSocket(addr) {};
	~AsyncTCPHTTPListenSocket() {};

	virtual AsyncTCPDataSocket* MakeNewSocket(int fd);
    };

    class AsyncTCPDataSocket : public AsyncTCPSocket {
    protected:
	AsyncTCPDataSocket(){};
	~AsuncTCPDataSocket(){};
	AsyncTCPDataSocket(int fd) { set_socket(fd); };
    public:
	// event handler
	int OnWritable();
	int OnError();
	int OnReadable();
    public:
	// operations initiated by upper layer
	int SendMessage(Buffer *msg, int msg_len);
	int ConnectTo(Address &to, Addresss, &from);

	// messagize
	virtual int Messagize(Buffer **msg, int *msg_len) = 0;
	
    private:
	Buffer *msg_in_recv_;
	int msg_size_;

	Buffer *send_buffer_list_;
    };

    class UDPSocket : public AsyncSocket {
    public:
	int BindOn(Address &addr);
	
    private:
    };

    // socket with packetization
    class LVSocket : public AsyncTCPDataSocket {
    public:
    private:
    };
    class HTTPSocket : public AsyncTCPDataSocket {
    };
    class LineSocket : public AsyncTCPDataSocket {
    };

    
};

#endif
