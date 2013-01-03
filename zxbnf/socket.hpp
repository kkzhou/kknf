
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

nanmespace ZXBNF {

    class Address {
    public:
	static Address* MakeAddress(char *ipstr, unsigned short port) {

	    struct sockaddr_in addr;
	    socklen_t addr_len;
	    bool valid = false;

	    memset(&addr, 0, sizeof(addr_));
	    addr.sin_len = sizeof(addr_) - 8;
	    addr.sin_family = PF_INET;
	    addr.sin_port = htons(port);

	    if (inet_aton(ipstr, &addr.sin_addr) != 1) {
		return 0;
	    }
	    addr_len = sizeof(struct sockaddr);
	    Address *new_address = new Address(addr, addr_len);
	    return new_address;

	};
	Address(struct sockaddr_in addr, socklen_t addr_len) {
	    addr_ = addr;
	    addr_len_ = addr_len;
	};
	Address& operator=(Address &addr) {
	    addr_ = addr;
	    addr_len_ = addr_len;
	    return *this;
	};

	inline struct sockaddr_in &addr() const { return addr_; };
	inline socklen_t &addr_len() const { return addr_len_; };

    private:
	struct sockaddr_in addr_;
	socklen_t addr_len_;
    };

    class Socket {
    public:
	inline int socket() const { return socket_; };	
	inline bool &error() const { return error_; };
	void Close() { 
	    assert(socket_ >= 0);
	    close(socket_);
	};
    protected:
	inline bool SetNonBlock() {

	    int flags = fcntl(fd, F_GETFL, 0);
	    if (flags == -1) {
		return false;
	    }
	
	    if (flags & O_NONBLOCK) {
		return false;
	    }
	    flags |= O_NONBLOCK;
	    if (fcntl(fd, F_SETFL, flags) != -1) {
		return false;
	    }
	    return true;
	};

	Socket() : socket_(-1), error_(false) {};
	~Socket() { Close(); };
    protected:
	int socket_;
	bool error_;
    };

    class AsyncTCPDataSocket : public Socket {
    public:
	int Create() {
	    socket_ = socket(PF_INET, SOCK_STREAM, 0);
	    SetNonblock();
	    return socket_;
	};

	int AsyncSend(std::list<char*> &data_list, std::list<int> &size_list) {

	    if (socket_ < 0) {
		return -1;
	    }
	    int iov_cnt = data_list.length();
	    struct iovec *iov = new struct iovec[iov_cnt];
	    std::list<char*>::iterator it = data_list.begin();
	    std::list<unsigned int>::iterator it2 = size_list.begin();
	    for (int i = 0; i < iov_cnt; i++) {
		iov[i].iov_base = *(it++);
		iov[i].iov_len = *(it2++);
	    }

	    int ret = writev(socket_, iov, iov_cnt);
	    if (ret < 0) {
		if (errno != EAGAIN) {
		    ret = -2;
		}
	    }
	    delete[] iov;
	    return ret;

	};

	int AsyncReceive(std::list<char*> &data_list, std::list<int> &size_list) {

	    if (socket_ < 0) {
		return -1;
	    }
	    int iov_cnt = data_list.length();
	    struct iovec *iov = new struct iovec[iov_cnt];
	    std::list<char*>::iterator it = data_list.begin();
	    std::list<unsigned int>::iterator it2 = size_list.begin();
	    for (int i = 0; i < iov_cnt; i++) {
		iov[i].iov_base = *(it++);
		iov[i].iov_len = *(it2++);
	    }

	    int ret = readv(socket_, iov, iov_cnt);
	    if (ret < 0) {
		if (errno != EAGAIN) {
		    ret = -2;
		}
	    }
	    delete[] iov;
	    return ret;

	};

    protected:
	AsyncTCPDataSocket() {};
	~AsyncTCPDataSocket() {};	
    };

    class AsyncTCPListenSocket : public Socket {
    public:
	int Create() {
	    socket_ = socket(PF_INET, SOCK_STREAM, 0);
	    SetNonblock();
	    return socket_;
	};
	AsyncTCPListenSocket() {};
	~AsyncTCPListenSocket() {};
	int AsyncListenOn(Address const &onaddr) {
	    if (socket_ <= 0) {
		return -1;
	    }

	    int ret = bind(socket_, &onaddr.addr(), onaddr.addr_len());
	    if ( ret == -1) {
		return -2;
	    }
	    ret = listen(socket_, 1024)
	    if ( ret == -1) {
		return -3;
	    }
	    return 0;

	};

	int AsyncAccept(AsyncTCPServerSocket **new_socket) {

	    assert(new_socket);
	    *new_socket = 0;
	    if (socket_ < 0) {
		return -2;
	    }

	    socklen_t addr_len;
	    struct sockaddr_in addr;

	    int ret = accept(socket_, (struct sockaddr*)&addr, &addr_len);
	    if (ret < 0) {
		if (errno != EAGAIN) {
		    return -2;
		}
		return -3;
	    }
	    *new_socket = new AsyncTCPServerSocket(ret);
	};

    private:
	// forbid
	AsyncTCPListenSocket& operator=(AsyncTCPListenSocket&) {};
	AsyncTCPListenSocket(AsyncTCPListenSocket&) {};
    };


    class AsyncTCPServerSocket : public AsyncTCPDataSocket {
    public:
	AsyncTCPServerSocket(int socket) : socket_(socket);
	~AsyncTCPServerSocket();
    private:
	AsyncTCPServerSocket() {};
    private:
	int socket_;
    };

    class AsyncTCPClientSocket : public AsyncTCPDataSocket {
    public:
	AsyncTCPClientSocket() {};
	~AsyncTCPClientSocket() {};
	int AsyncConnectTo(Address &toaddr) {
	    if (socket_ < 0) {
		return -2;
	    }

	    int ret = connect(socket_, (struct sockaddr*)&toaddr.addr(), toaddr.addr_len());
	    if (ret < 0) {
		if (errno != EAGAIN) {
		    return -3;
		}
		return 1;
	    }
	    return 0;
	};
    };

    class AsyncUDPDataSocket : public Socket {
    public:
	AsyncUDPDataSocket(){};
	~AsyncUDPDataSocket(){};
	int Create() {
	    socket_ = socket(PF_INET, SOCK_DATAGRAM, 0);
	    SetNonblock();
	    return socket_;
	};

	int AsyncReceiveFrom(char *data, int size, Address **fromaddr) {

	    assert(fromaddr);
	    int ret = 0;
	    if (*fromaddr) {
		ret = recvfrom(socket_, data, size, 0, (*fromaddr)->addr(), 
			       (*fromaddr)->addr_len());
		if (ret < 0) {
		    if (errno != EAGAIN || errno != EINTR) {
			return -2;
		    }
		}
		return 0;
	    } else {
		struct sockaddr_in addr;
		socklen_t addr_len;
		ret = recvfrom(socket_, data, size, 0, (struct sockaddr*)addr, &addr_len);
		if (ret < 0) {
		    if (errno != EAGAIN || errno != EINTR) {
			return -2;
		    }
		}
		return 0;
	    }
	    return 0;
	};
	int AsyncSendTo(char *data, int size, Address &toaddr) {

	    assert(type_ == T_UDP);
	    if (socket_ <= 0 ) {
		return -1;
	    }

	    int ret = sendto(socket_, data, size, 0, (struct sockaddr*)&toaddr.addr(), 
			     toaddr.addr_len());
	    if (ret < 0) {
		return -2;
	    }
	    return 0;
	};
    };
    class AsyncUDPServerSocket : public AsyncUDPDataSocket {
    public:
	int AsyncBindOn(Address &onaddr) {
	    if (socket_ < 0) {
		return -2;
	    }
	    int ret = bind(socket_, (struct sockaddr*)&onaddr.addr(), onaddr.addr_len());
	    if (ret < 0) {
		return -3;
	    }
	    return 0;
	};

    };
    class AsyncUDPClientSocket : public AsyncUDPDataSocket {
    };
};
#endif
