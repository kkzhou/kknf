#ifndef __ASYNC_TCP_SOCKET_HPP__
#define __ASYNC_TCP_SOCKET_HPP__

#include "async_socket.hpp"

namespace ZXBNF {

    class AsyncTCPSocket : public AsyncSocket {
    protected:
	// constructors & destructors
	AsyncTCPSocket() {};
	int BindOn(Address &addr) {
	    int fd = socket(PF_INET, SOCK_STREAM, 0);
	    if (fd  < 0) {
		return -1;
	    }
	    set_fd(fd);
	    Nonblock();
	    if (bind(fd, (struct sockaddr*)&addr.addr(), addr.addr_len()) < 0) {
		return -2;
	    }
	    return 0;
	};
    private:
	// prohibits
	AsyncTCPSocket(AsyncTCPSOcket&){};
	AsyncTCPSocket& operator=(AsyncTCPSocket&){};
    };

    class AsyncTCPListenSocket : public AsyncTCPSocket {
    protected:
	AsyncTCPListenSocket() {};
	~AsyncTCPListenSocket() {};
	int Linsten() {
	    if (listen(fd(), 1024) < 0) {
		return -1;
	    }
	    return 0;
	};
	int OnAcceptable(AsyncTCPDataSocket **newsk) {
	    Addresss addr;
	    int newfd = accept(fd(), (struct sockaddr*)&addr.addr(), &addr.addr_len());
	    if (newfd < 0) {
		return -1;
	    }
	    *newsk = MakeNewSocket(newfd);
	    return 0;
	};
	virtual AsyncTCPDataSocket* MakeNewSocket(int fd) = 0;
    private:
    };


    class AsyncTCPDataSocket : public AsyncTCPSocket {
    protected:
	AsyncTCPDataSocket(){};
	~AsuncTCPDataSocket(){};
	AsyncTCPDataSocket(int fd) { set_fd(fd); };
    public:
	// event handler
	// return value:
	// 0: OK but not complete, so continue
	// -1: error happens
	// >0: OK and complete n bytes
	int OnWritable();
	int OnError();
	int OnReadable();
    public:
	// operations initiated by upper layer
	int SendMessage(Buffer *msg, int msg_len) {
	    
	};
	int ConnectTo(Address &to, Addresss, &from);
	int ConnectTo(Address &to);

	// messagize
	virtual int Messagize(Buffer **msg, int *msg_len) = 0;
	
    private:
	Buffer *msg_in_recv_;
	int msg_size_;

	Buffer *send_buffer_list_;
    };


};
#endif
