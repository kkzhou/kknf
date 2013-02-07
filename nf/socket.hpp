#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

namespace NF {

    class Messagizor {
    public:
	Messagizizor(TCPSocket *sk);
	virtual bool IsComplete();
	virtual int MessageSize();
    private:
	TCPSocket *socket_;
    };
    class LVMessagizor : public Messagizor {
    public:
	LVMessagizor(TCPSocket *sk) : Messagizor(sk) {};
	virtual bool IsComplete() {
	};
	virtual int MessageSize() {
	};
    };

    class TCPSocket {
    public:
	TCPSocket(int fd);
	TCPSocket();
	~TCPSocket();
	bool ConnectTo(struct sockaddr_in &addr);
	int AsyncSend(char *buf, int size);
	int Close();
    public:
	int OnWritable();
	int OnReadable();
	int OnError();
    private:
	friend class Messagizor;
	int fd_;
	char *recv_buf_;
	int recv_buf_size_;
	int recv_buf_cur_;
	std::list<char*> send_buf_list_;
	int send_buf_cur_;
    };

    class TCPListener {
    public:
	TCPListener();
	~TCPListener();
	bool ListenOn(struct sockaddr_in &addr);
    public:
	int OnAcceptable();
    private:
	int fd_;
    };

    class UDPSocket {
    public:
	UDPSocket(struct sockaddr_in &addr);
	UDPSocket();
	~UDPSocket();
	int BindOn();
	int AsyncSend(struct sockaddr_in &to, char *buf, int size);
	int OnReadable();
	int OnWritable();
	int OnError();
	
    private:
	int fd_;
	std::list<char*> send_buf_list_;
	std::list<struct sockaddr_in> send_addr_list_;
    };
};

#endif
