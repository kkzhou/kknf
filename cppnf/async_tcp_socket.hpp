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
    public:
	struct TCPMessage {
	    unsigned long long id;
	    Buffer *data;
	};
    protected:
	AsyncTCPDataSocket(){};
	~AsuncTCPDataSocket(){ 
	    if (msg_in_recv_) {
		
		mempool()->ReturnBlock(msg_in_recv_->data);
	    }
	    std::list<TCPMessage*>::iterator it = send_msg_list_.begin();
	    std::list<TCPMessage*>::iterator endit = send_msg_list_.end();
	    for (; it != endit;) {
		mempool()->ReturnBlock((*it)->data);
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
	    if (send_msg_list_.size() == 0) {
		return -1;
	    }
	    struct iovec *v = new struct iovec[send_msg_list_.size()];
	    std::list<TCPMessage*>::iterator it = send_msg_list_.begin();
	    std::list<TCPMessage*>::iterator endit = send_msg_list_.end();
	    for (int i = 0; it != endit; it++, i++) {
		v[i].iov_base = (*it)->data->start + (*it)->data->head;
		v[i].iov_len = (*it)->data->tail - (*it)->data->head;
	    }
	    assert(i == send_msg_list_.size());
	    int ret = writv(fd(), v, i);
	    if (ret < 0) {
		delete[] v;
		return -1;
	    } 
	    if (ret == 0) {
		delete[] v;
		return -2;
	    }
	    int left = ret;
	    it = send_msg_list_.begin();
	    endit = send_msg_list_.end();
	    for (; it != endit, left > 0;) {
		Buffer *buf = (*it)->data;
		int len = buf->tail - buf->head;
		if (len <= left) {
		    mempool()->ReturnBlock(buf);
		    it = send_msg_list_.erase(it);
		    left -= len;
		} else {
		    buf->head += left;
		    left = 0;
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
		assert(msg_in_recv_size_ == 0);
		msg_in_recv_ = new TCPMessage;
		msg_in_recv_->data = mempool()->GetSmallBlock();
	    }

	    int ret = 0;
	    ExtractMessageSize();
	    while (true) {
		Buffer *buf = msg_in_recv_->data;
		int filled = buf->tail();
		int left = buf->length() - buf->tail();
		if (msg_in_recv_size_ > 0) {
		    if (left + filled < msg_in_recv_size_) {
			if (msg_in_recv_size_ > mempool()->large_block_size()) {
			    ret = -1;
			    break;
			}
			Buffer *newbuf = mempool()->GetLargeBlock();
			
			memcpy(newbuf->start(), buf->start() + buf->head(), 
			       buf->tail() - buf->head());
			newbuf->tail =  buf->tail - buf->head;
			mempool()->PutBlock(buf);
			msg_in_recv_->data = newbuf;
			left = msg_in_recv_->data->length - msg_in_recv_->data->tail;
		    }
		}
		buf = msg_in_recv_->data;
		int num = read(fd(), buf->start() + buf->tail(), left);
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
		if (msg_in_recv_size_ == buf->tail() - buf->head) {
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
	unsigned long long SendMessage(Buffer *buf) {
	    if (send_msg_list_.size() == kMaxSendBufferNum) {
		return 0;
	    }

	    TCPMessage *newmsg = new TCPMessage;
	    newmsg->data = buf;
	    newmsg->id = GetID();
	    send_msg_list_.push_back(newmsg);
	    return newmsg->id;
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
		if (errno == EAGAIN) {
		    return 1;
		}
		return -1;
	    }
	    return 0;
	};

	// messagize
	virtual int ExtractMessageSize() = 0;

	AsyncTCPDataSocket::TCPMessage* GetMessage() {
	    if (msg_in_recv_->data->tail - msg_in_recv_->data->head == msg_in_recv_size_) {
		TCPMessage *ret = msg_in_recv_;
		msg_in_recv_ = 0;
		msg_in_recv_size_ = 0;
		return ret;
	    }
	    return 0;
	};
    public:
	unsigned long long GetID() { retur cur_id_++; };
    protected:
	TCPMessage *msg_in_recv_;
	int msg_in_recv_size_;
    private:
	std::list<TCPMessage*> send_msg_list_;
    private:
	unsigned long long cur_id_;
	static const int kMaxSendBufferNum = 1000;
    };

};
#endif
