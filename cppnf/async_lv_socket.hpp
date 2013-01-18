#ifndef __ASYNC_LV_SOCKET_HPP__
#define __ASYNC_LV_SOCKET_HPP__


#include "async_tcp_socket.hpp"

namespace ZXBNF {

    class LVListenSocket : public AsyncTCPListenSocket {
    public:
	LVListenSocket() {};
	~TCPLVListenSocket() {};

	virtual AsyncTCPDataSocket* MakeNewSocket(int fd);
    };

    class LVSocket : public AsyncTCPDataSocket {
    public:
	virtual int Messagize(Buffer **msg, int *msg_len);
    };
};

#enidf
