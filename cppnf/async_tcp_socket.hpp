#ifndef __ASYNC_TCP_SOCKET_HPP__
#define __ASYNC_TCP_SOCKET_HPP__

#include "async_socket.hpp"

namespace ZXBNF {

    class AsyncTCPSocket : public AsyncSocket {
    protected:
	// constructors & destructors
	AsyncTCPSocket() {};
	int BindOn(Address &addr);
    private:
	// prohibits
	AsyncTCPSocket(AsyncTCPSOcket&){};
	AsyncTCPSocket& operator=(AsyncTCPSocket&){};
    };

    class AsyncTCPListenSocket : public AsyncTCPSocket {
    protected:
	AsyncTCPListenSocket() {};
	~AsyncTCPListenSocket() {};
	int Linsten();
	int OnAcceptable();
	virtual AsyncTCPDataSocket* MakeNewSocket(int fd);
    private:
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
