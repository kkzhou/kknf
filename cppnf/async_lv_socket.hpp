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
	    if (msg_in_recv_->tail() - msg_in_recv_->head() >= 4) {
		msg_size_ = htonl(*(int*)(msg_in_recv_->start() + msg_in_recv_->head()));
		return msg_size_;
	    }
	    return 0;
	};
    };
};

#enidf
