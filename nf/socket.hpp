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

    class TCPSocket : public EventListener {
    public:
	struct TCPMessage {
	    char *data;
	    int size;	    
	};
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
	TCPMessage *receive_msg_;
	int received_;
	std::list<TCPMessage*> send_msg_list_;
	int sent_;
    };

    class TCPListener : public EventListener {
    public:
	TCPListener();
	~TCPListener();
	bool ListenOn(struct sockaddr_in &addr) {
	    int fd = socket(PF_INET, SOCK_DATAGRAM, 0);
	    if (fd < 0) {
		return false;
	    }
	    set_fd(fd);
	    if (bind(fd(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		return false;
	    }
	    return true;
	};
    public:
	int OnAcceptable();
    };

    class UDPSocket : public EventListener {
    public:
	struct UDPMessage {
	    char *data;
	    int size;
	    struct sockaddr_in to;
	};
    public:
	UDPSocket();
	~UDPSocket();
	int BindOn(struct sockaddr_in &addr);
	int AsyncSend(struct sockaddr_in &to, char *buf, int size);
    public:
	virtual int Handler() {
	    
	};
	int OnReadable();
	int OnWritable();
	int OnError();
	
    private:
	std::list<UDPMessage*> send_msg_list_;
    };
};

#endif
