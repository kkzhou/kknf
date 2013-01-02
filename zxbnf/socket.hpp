
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

nanmespace ZXBNF {

    enum SocketType {
	T_INVALID = 1,
	T_UDP,
	T_TCP,	
	T_LISTENER
    };

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

	struct sockaddr_in &addr() const { return addr_; };
	socklen_t &addr_len() const { return addr_len_; };

    private:
	struct sockaddr_in addr_;
	socklen_t addr_len_;
    };

    class Socket {
    public:
	Socket(SocketType t, int socket = -1) : 
	    socket_(socket), 
	    type_(t), 
	    connected_(false) {
	    if (socket_ <= 0) {
		if (type_ == T_UDP) {
		    socket_ = socket(AF_INET, SOCK_DATAGRAM, 0);
		} else {
		    socket_ = socket(AF_INET, SOCK_STREAM, 0);
		}
	    }
	};

	int AsyncConnectTo(Address const &toaddr) {

	    assert(type_ == T_TCP);
	    if (socket_ < 0) {
		return -1;
	    }
	
	    int ret = connect(socket_, &toaddr.addr(), toaddr.addr_len());
	    if ( ret == -1) {
		if (errno != EAGAIN || errno != EINPROCESS || errno != EINTR) {
		    return -2;
		}
		return -3;
	    } 
	    connected_ = true;
	    return 0;
		    	    
	};

	int AsyncLinstenOn(Address const &onaddr) {

	    assert(type_ == T_LISTENER);
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

	int AsyncAccept(Socket **new_socket) {

	    assert(type_ == T_LISTENER);
	    if (socket_ <= 0) {
		return -1;
	    }
	    
	    *new_socket = 0;
	    socklen_t addr_len;
	    struct sockaddr_in addr;

	    int ret = accept(socket_, (struct sockaddr*)&addr, &addr_len);
	    if (ret < 0) {
		if (errno != EAGAIN) {
		    return -2;
		}
		return -3;
	    }
	    *new_socket = new Socket(T_TCP, ret);
	    (*new_socket)->connected() = true;
	}; 
       	
	bool SetNonBlock() {

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

	int AsyncSend(std::list<char*> data_list, 
			       std::list<unsigned int> size_list) {

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

	int AsyncSendTo(Address const &toaddr, char *data, unsigned int size) {
	    
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

	int AsyncReceive(std::list<char*> data_list, std::list<unsigned int> size_list) {
	    
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

	int AsyncReceiveFrom(char *data, unsigned int size, Address &from_addr)

	    assert(type_ == T_UDP);
	    if (socket_ <= 0 ) {
		return -1;
	    }

	    int ret = recvfrom(socket_, data, size, 0, (struct sockaddr*)&from_addr.addr(), 
			     from_addr.addr_len());
	    if (ret < 0) {
		return -2;
	    }
	    return 0;

	};

	Address* AsyncReceiveFrom(char *data, unsigned int size)

	    assert(type_ == T_UDP);
	    if (socket_ <= 0 ) {
		return 0;
	    }
	    struct sockaddr_in from_addr;
	    socklen_t from_addr_len;
	    int ret = recvfrom(socket_, data, size, 0, (struct sockaddr*)&from_addr, 
			     from_addr_len);
	    if (ret < 0) {
		return 0;
	    }
	    Address *new_address = Address::MakeAddress(T_UDP, from_addr, from_addr_len);
	    return new_address;
	};

	bool &connected() { return connected_; };
	int socket() const { return socket_; };

    private:
	
	Socket& operator=(Socket&) {};
	Socket(Socket&) {};

	int socket_;
	SocketType type_;
	bool connected_;
    };

};
#endif
