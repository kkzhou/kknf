#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "socket.hpp"

namespace ZXBNF {

    class TCPMessage {
    public:
	TCPMessage():
	    head_(0),
	    tail_(0),
	    message_size_(0),
	    cur_(0),
	    in_recv_(true){};

	TCPMessage(bool in_recv):
	    head_(0),
	    tail_(0),
	    message_size_(0),
	    cur_(0),
	    in_recv_(in_recv){};

	inline int &message_size() { return message_size_; };
	inline int SeekForward(int num) {
	    assert(cur_);
	    // check length
	    int left = 0;
	    Buffer *iter = cur_;
	    left = num;
	    while (cur_ && left > 0) {
		if (cur_->tail() - cur_->head() <= left) {
		    left -= cur_->tail() - cur_->head();
		    cur_->head() = cur_->tail();
		    cur_ = cur_->next();
		} else {
		    cur_->head() += left;
		    left = 0;		    
		}
	    }
	    return num - left;
	};
	inline int FillForward(int num) {
	    
	    assert(cur_);
	    // check length
	    int left = 0;
	    Buffer *iter = cur_;
	    while (iter) {
		left += iter->length() - iter->tail();
	    }
	    if (left < num) {
		return -1;
	    }
	    left = num;
	    while (left > 0) {
		if (cur_->length() - cur_->tail() <= left) {
		    left -= cur_->length() - cur_->tail();
		    cur_->tail() = cur_->length();
		    cur_ = cur_->next();
		} else {
		    cur_->tail() += left;
		    left = 0;		    
		}
	    }
	    return 0;
	};
	inline Buffer *head() { return head_; };
	inline Buffer* &cur() { return cur_; }

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
	Buffer *cur_;
	bool in_recv_;
    };
    class TCPMessageForRecveive : public TCPMesage {
    };
    class TCPMessageForSend : public TCPMesage {
    public:
	TCPMesageForSend() : TCPMessage(false) {};
    };

    class TCPConnection {
    public:
	TCPConnection() :
	    socket_(0),
	    message_size_(-1) {
	};
    public:
	inline int &socket() { return socket_; };
	inline int ShrinkSendMsgList(int num) {
	    std::list<TCPMessageForSend*>::iterator iter = send_msg_list_.begin();
	    std::list<TCPMessageForSend*>::iterator end_iter = send_msg_list_.end();
	    int left = num;
	    while (iter != end_iter) {
		int reduced = (*iter)->SeekForward(left);
		assert(reduced > 0);
		if (reduced == left) {
		    
		}
	    }
	};
	inline int GetIOVectorToFill(struct iov **iovec) {
	    
	};
    private:
	int socket_;
	std::list<TCPMessageForReceive*> recv_msg_list_;
	std::list<TCPMessageForSend*> send_msg_list_;
	TCPMessageForSend *send_msg_;
	TCPMessageForReceive *recv_msg_;
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
