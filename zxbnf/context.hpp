#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "socket.hpp"

namespace ZXBNF {
    class TCPContext {
    public:
	class TCPConnection {
	public:
	    TCPConnection() : 
		from_socket_(0),
		recv_buffer_num_(0), 
		cur_buffer_to_fill_(0),
		message_size_(-1) {
	    };
	public:
	    inline AsyncTCPServerSocket* &from_socket() { return from_socket_; };
	    inline std::list<Buffer*> &recv_buffer_list() { return recv_buffer_list_; };
	    inline std::list<Buffer*> &send_buffer_list() { return send_buffer_list_; };
	    inline std::vector<Buffer*> &ongoing_recv_buffer() { return ongoing_recv_buffer_; };
	    inline int &cur_buffer_to_fill() { return cur_buffer_to_fill_; };
	    inline int &received_byte_num() { return  recveived_byte_num_; };
	    inline int &message_size() {return message_size_; };
	private:
	    AsyncTCPServerSocket *from_socket_;
	    std::list<Buffer*> recv_buffer_list_;
	    std::list<Buffer*> send_buffer_list_;
	    std::vector<Buffer*> ongoing_recv_buffer_;
	    int cur_buffer_to_fill_;
	    int recveived_byte_num_;
	    int message_size_;
	};
	TCPContext(MemPool *pool) : mempool_(pool) {};
    public:
	int DataArrivedCallback(AsyncTCPServerSocket *from_socket) {
	    assert(false);
	};
	int MessageArrivedCallback(AsyncTCPServerSocket *from_socke, 
				   std::list<Buffer*> &data) {
	    assert(false);
	};
	int ErrorArrivedCallback(AsyncTCPServerSocket *from_socket) {
	    all_connection_[from_socket->socket()] = -1;
	    from_socket->Close();
	    from_socket->error()  = true; // NOT remove from 'socket_in_use_'
	    socket_to_close_.push_back(from_socket);
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
