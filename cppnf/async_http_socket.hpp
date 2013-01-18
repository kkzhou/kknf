#ifndef __ASYNC_HTTP_SOCKET_HPP__
#define __ASYNC_HTTP_SOCKET_HPP__


#include "async_tcp_socket.hpp"

namespace ZXBNF {

    class HTTPListenSocket : public AsyncTCPListenSocket {
    public:
	HTTPListenSocket() {};
	~HTTPListenSocket() {};

	virtual AsyncTCPDataSocket* MakeNewSocket(int fd);
    };

    class HTTPSocket : public AsyncTCPDataSocket {
    public:
	virtual int Messagize(Buffer **msg, int *msg_len);
    };
};

#enidf
