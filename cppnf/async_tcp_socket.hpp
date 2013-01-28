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
	~AsuncTCPDataSocket(){ 
	    while (msg_in_recv_) {
		Buffer *buf = msg_in_recv_.next();
		mempool()->ReturnBlock(msg_in_recv_);
		msg_in_recv_ = buf;
	    }
	    while (send_buffer_list_) {
		Buffer *buf = send_buffer_list_.next();
		mempool()->ReturnBlock(send_buffer_list_);
		send_buffer_list_ = buf;
	    }
	};
	AsyncTCPDataSocket(int fd) { set_fd(fd); };
    public:
	// event handler
	// return value:
	// 0: OK but not complete, so continue
	// -1: error happens
	// 1: OK and complete
	int OnWritable() {
	    struct iovec *v = new struct iovec[send_buffer_num_];
	    Buffer *buf = send_buffer_head_;
	    for (int i = 0; i < send_buffer_num_; i++) {
		v[i].iov_base = buf->start() + buf->head();
		v[i].iov_len = buf->tail() - buf->head();
	    }
	    assert(i == send_buffer_num_);
	    int ret = writv(fd(), v, send_buffer_num_);
	    if (ret < 0) {
		delete v;
		return -1;
	    } else if (ret == 0) {
		delete v;
		return -2;
	    } else {
		
	    }
	};
	int OnError();
	int OnReadable();
    public:
	// operations initiated by upper layer
	int SendMessage(Buffer *msg) {
	    if (send_buffer_num_ == kMaxSendBufferNum) {
		return -1;
	    }
	    if (send_buffer_tail_) {
		send_buffer_tail_->next() = msg;
		msg->next() = 0;
		msg->prev() = send_buffer_tail_;
		send_buffer_tail_ = msg;
	    } else {
		assert(send_buffer_head_ == 0);
		assert(send_buffer_num_ == 0);
		send_buffer_head_ = send_buffer_tail_ = msg;
		msg->next() = msg->prev() = 0;
	    }
	    return ++send_buffer_num_;
	};
	int ConnectTo(Address &to, Addresss, &from);
	int ConnectTo(Address &to);

	// messagize
	virtual int Messagize(Buffer **msg, int *msg_len) = 0;
	
    private:
	Buffer *msg_in_recv_;
	int msg_size_;

	Buffer *send_buffer_head_;
	Buffer *send_buffer_tail_;
	int send_buffer_num_;
	static const int kMaxSendBufferNum = 1000;
    };


};
#endif
