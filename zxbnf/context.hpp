#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "socket.hpp"

namespace ZXBNF {

    class TCPMessage {
    protected:
	TCPMessage():
	    head_(0),
	    tail_(0),
	    message_size_(-1),
	    message_left_(-1),
	    cur_(0) {};
    public:
	inline int message_size() { return message_size_; };
	inline int message_left() { return message_left_; };
	inline Buffer *head() { return head_; };
	inline Buffer* &cur() { return cur_; }

    protected:
	inline Buffer* Reset()  {
	    Buffer *ret = head_;
	    head_ = cur_ = tail_ = 0;
	    message_size_ = message_left_ = -1;
	    return ret;
	};
	inline int SetMessageSize(int size) { assert(false); };
	inline int Forward(int num) { assert(false); };


	inline Buffer* AppendBuffer(Buffer *buffer) {

	    assert(buffer);
	    assert(buffer->prev() == 0);
	    assert(buffer->next() == 0);

	    if (head_ == 0) {
		buffer->next() = 0;
		buffer->prev() = 0;
		head_ = tail_ = buffer;
		cur_ = head_;
	    } else {
		assert(tail_);
		tail_->next() = buffer;
		buffer->prev() = tail_;
		tail_ = tail_->next();
	    }
	    return head_;
		
	};
    private:
	Buffer *head_;	// Maybe not 'one' buffer but a list
	Buffer *tail_;
	int message_size_;
	int message_left_;
	Buffer *cur_;
    };

    class TCPMessageForRecveive : public TCPMesage {
    public:
	TCPMessageForReceive() {};
	inline int Forward(int num) {
	    
	    assert(cur());
	    // check length
	    int left = 0;
	    Buffer *iter = cur();
	    while (iter) {
		left += iter->length() - iter->tail();
	    }
	    if (left < num) {
		return -1;
	    }
	    left = num;
	    while (left > 0) {
		if (cur()->length() - cur()->tail() <= left) {
		    left -= cur()->length() - cur()->tail();
		    cur()->tail() = cur()->length();
		    cur() = cur()->next();
		} else {
		    cur()->tail() += left;
		    left = 0;		    
		}
	    }
	    message_left_ -= num;
	    return 0;
	};
	inline int SetMessageSize(int size) {
	    assert(message_size_ == -1);
	    assert(size > 0);
	    message_size_ = size;
	    message_left_ = size;
	    Buffer *it = head_;
	    while (it && (*it)->tail() != (*it)->length()) {
		message_left_ -= (*it)->length() - (*it)->tail();
		it++;
	    }
	    return message_left_;
	};
    private:
	TCPMessageForReceive(TCPMessageForReceive&){};
	TCPMessageForReceive& operator=(TCPMessageForReceive&){};
    };

    class TCPMessageForSend : public TCPMessage {
    public:
	TCPMesageForSend(){};
	inline bool AllSent() { return cur() == 0; };
	inline int Forward(int num) {
	    assert(cur());
	    // check length
	    int left = 0;
	    Buffer *iter = cur();
	    left = num;
	    while (cur() && left > 0) {
		if (cur()->tail() - cur()->head() <= left) {
		    left -= cur()->tail() - cur()->head();
		    cur()->head() = cur()->tail();
		    cur() = cur()->next();
		} else {
		    cur()->head() += left;
		    left = 0;		    
		}
	    }
	    return num - left;
	};

	inline int SetMessageSize(int size) {
	    assert(message_size_ == -1);
	    assert(size > 0);
	    message_size_ = size;
	    message_left_ = size;
	    Buffer *it = head_;
	    while (it && (*it)->head() != (*it)->tail()) {
		message_left_ -= (*it)->tail() - (*it)->head();
		it++;
	    }
	    return message_left_;
	};

    private:
	// forbid
	TCPMessageForSend(TCPMessageForSend&) {};
	TCPMessageForSend& operator=(TCPMessageForSend&) {};

    };

    class TCPConnection {
    public:
	TCPConnection(MemPool *pool) :
	    socket_(0),
	    pool_(pool) {
	};

    public:
	inline int &socket() { return socket_; };

	inline int ShrinkSendMsgList(int num) {

	    std::list<TCPMessageForSend*>::iterator iter = send_msg_list_.begin();
	    std::list<TCPMessageForSend*>::iterator end_iter = send_msg_list_.end();
	    int left = num;
	    while (iter != end_iter) {
		int reduced = (*iter)->SeekForward(left);

		if ((*iter)->AllSent()) {
		    Buffer *to_release = (*iter)->Reset();
		    while (to_release) {
			Buffer *it = to_release->next();
			pool_->PutBlock(to_release);
			to_release = it;
		    } // while
		    iter = send_msg_list_.remove(iter);
		} else {
		    iter++;
		}
		left -= reduced;
		if (left == 0) {
		    break;
		}
	    } // while
	    return left;
	};
	inline TCPMessage* PopReceiveMessage() {
	    TCPMessage *ret = recv_msg_list_.front();
	    recv_msg_list_.pop_front();
	};
	inline int PushMessageToSend( ) {
	};
	inline int GetIOVectorToFill(struct iov *iovec, int iov_num) {
	    std::list<TCPMessageForSend*>::iterator iter = send_msg_list_.begin();
	    std::list<TCPMessageForSend*>::iterator end_iter = send_msg_list_.end();

	    int i = 0;
	    while (iter != end_iter) {
		Buffer *it = (*iter)->head();
		while (it) {
		    if (it->tail() == it->head()) {
			continue;
		    }
		    iovec[i].iov_base = it->start() + it->head();
		    iovec[i].iov_len = it->tail();
		    i++;
		    if (i == iov_num) {
			break;
		    }
		    it++;
		}
	    } // while
	    return i;
	};
    private:
	int socket_;
	std::list<TCPMessageForReceive*> recv_msg_list_;
	std::list<TCPMessageForSend*> send_msg_list_;
	MemPool *pool_;
    };

    class TCPContext {
    public:
	TCPContext(MemPool *pool) : mempool_(pool) {};
    public:
	int DataArrivedCallback(int socket) {
	    assert(false);
	};
	int MessageArrivedCallback(int socket, std::list<Buffer*> &data) {
	    assert(false);
	};
	int ErrorArrivedCallback(int socket) {
	    Socket *sk = all_connection_[from_socket->socket()] = 0;
	    sk->Close();
	    sk->error()  = true; // NOT remove from 'socket_in_use_'
	    socket_to_close_.push_back(sk);
	};

	std::vector<TCPConnection*> const &all_connection() { 
	    return all_connection_; 
	};
	std::list<AsyncTCPServerSocket*> const &socket_in_use() { 
	    return socket_in_use_; 
	};
	std::list<AsyncTCPServerSocket*> const &socket_to_close() { 
	    return socket_to_close_; 
	};
	MemPool* mempool() { return mempool_; };
    private:
	AsyncTCPListenSocket *listner_;
	MemPool mempool_;
	// connection management
	std::vector<TCPConnection*> all_connection_;
	std::list<AsyncTCPServerSocket*> socket_in_use_;
	std::list<AsyncTCPServerSocket*> socket_to_close_;
    };

    class TCPLVContext : public TCPContext {
    public:

	int DataArrivedCallback(AsyncTCPServerSocket *from_socket) {

	    TCPConnection *conn = 0;
	    bool error = false;
	    try {
		conn = all_connection().at(from_socket->socket());
	    } catch (std::out_fo_range &e) {
		
		error = true;
		
	    }
	    if (error) {
		return -1;
	    }

	    if (conn->ongoing_recv_buffer().empty()) {
		// a new message
		assert(conn->message_size() == 0);
		assert(conn->recveived_byte_num() == 0);
		assert(conn->cur_buffer_to_fill() == 0);
		conn->ongoing_recv_buffer().push_back(mempool()->GetSmallBlock());
	    }

	    Buffer *h = *(conn->ongoing_recv_buffer().begin());
	    int L = -1;
	    struct iovec *iov;
	    int iov_num = 0;

	    if (h->end() >= 4) {
		// 'length' field has been read
		iov = new struct iovec[conn->ongoing_recv_buffer().size()];
		L = *(int*)(h->start());

		for (int cnt = cur_buffer_to_fill(); cnt < ongoing_recv_buffer().size(); 
		     cnt++) {
		    Buffer *it = ongoing_recv_buffer().at(cnt);
		    iov[iov_num].iov_base = it->start() + it->used();
		    iov[iov_num].iov_len = it->length() - it->used();;
		    iov_num++;
		}
	    } else {
		// 'length' field hasn't been read
		iov = new struct iovec[1];
		iov[0].iov_base = h->start() + h->used();
		iov[0].iov_len = h->length() - h->used();
		iov_num = 1;
	    }

	    int ret = readv(from_socket->socket(), iov, iov_num);
	    if (ret < 0) {
		if (errno != EAGAIN && errno != EINTR) {
		    delete[] iov;
		    return -2;
		}
	    } else if (ret == 0) {
	    } else {
		// modify buffers
		int cur, size;
		cur = cur_buffer_to_fill();
		size = ongoing_recv_buffer().size();
		for (int i = ret, int cnt = cur; cnt < size; cnt++) {

		    Buffer *it = conn->ongoing_recv_buffer().at(cnt);
		    if (i > it->length()) {
			it->end() = it->length();
			i -= it->length();
		    } else {
			it->end() = i;
			i = 0;
			cur_buffer_to_fill() = cnt;
			conn->recveived_byte_num() += ret;
		    }
	    }
	
	};
	int MessageArrivedCallback(AsyncTCPServerSocket *from_socket, 
				   std::list<Buffer*> &data) {
	    assert(false);
	};
	private:
	const int kMaxMessageSize_ = 4 * 1024 * 1024;

    };

    class UDPContext {
    public:
	struct UDPData {
	    Address addr_;
	    std::list<Buffer*> recv_buffer_;
	};

    public:
	int DataArrivedCallback(UDPData *data) {
	    return MessageArrivedCallback(data);
	};
	int MessageArrivedCallback(UDPData *data) {
	    
	};
	
    private:
	AsyncUDPServerSocket *server_;
	UDPData *recv_data_;
	std::list<UDPData*> send_data_list_;
    };

    class RequestContext {
    };
};

#endif
