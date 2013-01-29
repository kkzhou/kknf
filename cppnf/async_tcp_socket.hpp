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
    private:
	virtual AsyncTCPDataSocket* MakeNewSocket(int fd) = 0;
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
	// 0: OK and continue
	// <0: error happens
	// 1: OK and complete(donnt epoll any more)
	// 2: OK and close
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
		delete[] v;
		return -1;
	    } 
	    if (ret == 0) {
		delete[] v;
		return -2;
	    }
	    int left = ret;
	    Buffer *old;
	    buf = send_buffer_head_;
	    old = buf;
	    while (left > 0) {
		int len = buf->tail() - buf->head();
		if (len <= left) {
		    buf = buf->next();
		    mempool()->PutBlock(old);
		    left -= len;
		    if (!buf) {
			break;
		    }
		    buf->prev() = 0;
		    old = buf;
		} else {
		    buf->head() += left;
		    left -= len;
		}
	    }
	    assert(left == 0);

	    if (state() == AsyncSocket::S_TO_CLOSE_AFTER_SEND_ALL_DATA
		|| state() == AsyncSocket::S_TO_CLOSE) {
		delete[] v;
		return 2;
	    }
	    delete[] v;
	    return 0;
	};

	int OnError() {
	    return 0;
	};
	// return value:
	// 2: OK, complete and close
	// 1: OK, continue
	// 0: OK, complete a message and continue
	// <0: error
	int OnReadable() {
	    if (msg_in_recv_ == 0) {
		assert(msg_size_ == 0);
		msg_in_recv_ = mempool()->GetSmallBlock();
		msg_in_recv_->next() = msg_in_recv_->prev() = 0;
	    }

	    int ret = 0;
	    while (true) {
		int msg_size = ExtractMessageSize();
		int filled = msg_in_recv_->tail();
		int left = msg_in_recv_->length() - msg_in_recv_->tail();
		if (msg_size > 0) {
		    if (left + filled < msg_size) {
			if (msg_size > mempool()->large_block_size()) {
			    ret = -1;
			    break;
			}
			Buffer *newbuf = mempool()->GetLargeBlock();
			newbuf->head() = newbuf->tail() = 0;
			memcpy(newbuf->start(), msg_in_recv_->start() + msg_in_recv_->head(), 
			       msg_in_recv_->tail() - msg_in_recv_->head());
			newbuf->tail() = msg_in_recv_->tail() - msg_in_recv_->head();
			mempool()->PutBlock(msg_in_recv_);
			msg_in_recv_ = newbuf;
			left = msg_in_recv_->length() - msg_in_recv_->tail();
		    }
		}

		int num = read(fd(), msg_in_recv_->start() + msg_in_recv_->tail(), left);
		if (num < 0) {
		    if (errno != EAGAIN || errno != EINTR) {
			ret = -2;
			break;
		    }
		    continue;
		}
		if (num == 0) {
		    ret = -3;
		    break;
		}
		if (msg_size_ == msg_in_recv_->tail() - msg_in_recv_->head()) {
		    if (state() == AsyncSocket::S_TO_CLOSE 
			|| state() == AsyncSocket::S_TO_CLOSE_AFTER_SEND_ALL_DATA) {
			ret = 2;
		    } else {
			ret = 0;
		    }
		    break;
		} else {
		    ret = 1;
		    break;
		}
	    } // while
	    return ret;
	};
	
    public:
	// operations initiated by upper layer
	int CloseAfterSendAllData() {
	    set_state(AsyncSocket::S_TO_CLOSE_AFTER_SEND_ALL_DATA);
	    return 0;
	};
	int CloseNow() {
	    set_state(AsyncSocket::S_TO_CLOSE);
	    return 0;
	};
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
	int ConnectTo(Address &to, Addresss, &from) {
	    if (BindOn(from) < 0) {
		return -1;
	    }
	    return ConnectTo(to);
	};
	int ConnectTo(Address &to) {
	    int ret = connect(fd(), (struct sockaddr*)&to.addr(), to.addr_len());
	    if (ret < 0) {
		return -2;
	    }
	    return 0;
	};

	// messagize
	virtual int MessageSize() = 0;

	Buffer* GetMessage() {
	    if (msg_in_recv_->tail() - msg_in_recv_->head() == msg_size_) {
		Buffer *ret = msg_in_recv_;
		msg_in_recv_ = 0;
		msg_size_ = 0;
		return ret;
	    }
	    return 0;
	};
    protected:
	Buffer *msg_in_recv_;
	int msg_size_;
    private:
	Buffer *send_buffer_head_;
	Buffer *send_buffer_tail_;
	int send_buffer_num_;
	static const int kMaxSendBufferNum = 1000;
    };


};
#endif
