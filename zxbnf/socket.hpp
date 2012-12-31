
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

nanmespace ZXBNF {

    class Address {
    public:
	Address() {
	    
	};

	Address(char *ipstr, unsigned short port) {
	    memset(&addr_, sizeof(addr_));
	    addr_.sin_len = sizeof(addr_) - 8;
	    addr_.sin_family = PF_INET;
	    addr_.sin_port = htons(port);
	    if (inet_aton(ipstr, &addr_.sin_addr) != 1) {
		valid_ = false;
	    }
	    addr_len_ = sizeof(struct sockaddr);
	};

	struct sockaddr addr() const { return addr_; };
	socklen_t addr_len() const { return addr_len_; };
	bool valid() const { return valid_; };

    private:
	struct sockaddr_in addr_;
	socklen_t addr_len_;
	bool valid_;
    };


    class Socket {
    public:
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
	    
	    int ret = sendto(socket_, data, size, 0, &toaddr.addr(), toaddr.addr_len());
	    if (ret < 0) {
		
	    }
	};

	// unsigned int SyncSend(std::list<char*> data_list, 
	// 		       std::list<unsigned int> size_list) {

	//     int iov_cnt = data_list.length();
	//     struct iovec *iov = new struct iovec[iov_cnt];
	//     std::list<char*>::iterator it = data_list.begin();
	//     std::list<unsigned int>::iterator it2 = size_list.begin();
	//     for (int i = 0; i < iov_cnt; i++) {
	// 	iov[i].iov_base = *(it++);
	// 	iov[i].iov_len = *(it2++);
	//     }
	//     int from = 0;
	//     int ret = 0;
	//     int retry = 0;
	//     int error = 0;
	//     while (retry++ < 3 && from < iov_cnt) {

	// 	ret = writev(socket_, &iov[from], iov_cnt - from);
	// 	if (ret < 0) {
	// 	    if (errno != EAGAIN) {
	// 		error = -2;
	// 		break;
	// 	    }
	// 	} 

	// 	while (ret > 0) {
	// 	    if (iov[from].iov_len > ret) {
	// 		iov[from].iov_base += ret;
	// 		ret = 0;
	// 	    } else if (iov[from].iov_len == ret) {
	// 		from++;
	// 		ret = 0;
	// 	    } else {
	// 		ret -= iov[from].iov_len;
	// 		from++;
	// 	    }
	// 	}

	//     } 

	//     delete[] iov;

	//     if (error == 0) {
	// 	i(from < iov_cnt)
	// 	    error = -3;
	//     }
	//     return error;

	// };
	unsigned int AsyncReceive(char *data, unsigned int size) {
	};
	// unsigned int SyncReceive(char *data, unsigned int size) {
	// };
	unsigned int AsyncReceiveFrom(unsigned int ip, unsigned short port, 
				      char *data, unsigned int size) {
	};

	bool &ready() { returean ready_; };
	int socket() { return socket_; };

    private:
	
	Socket& operator=(Socket&) {};
	Socket(Socket&) {};

    protected:
	Socket() : 
	    socket_(-1), 
	    type_(T_INVALID), 
	    ready_(false) {
	};

	int socket_;
	SocketType type_;
	bool ready_;
    
    };

    Socket* CreateListener(SocketType t, char const *ip, unsigned short port)
    {
    };
    Socket* CreateClient(SocketType t, char const *toip, unsigned short toport)
    {
    };

};
#endif
