#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

namespace NF {

    class Processor {
    public:
	virtual int ProcessNewConnection(int fd) = 0;
	virtual int ProcessMessage(int fd, char *data, int size) = 0;
	virtual int ProcessMessage(int fd, char *data, int size, struct sockaddr_in from) = 0;
    };

    class Messagizor {
    public:
	Messagizizor(TCPSocket *sk) : socket_(sk) {};
	virtual bool IsComplete() = 0;
	virtual int MessageSize() = 0;
	TCPSocket* socket() { return socket_; };
    private:
	TCPSocket *socket_;
    };

    class LVMessagizor : public Messagizor {
    public:
	LVMessagizor(TCPSocket *sk) : Messagizor(sk) {};
	virtual bool IsComplete() {
	    return socket()->receive_msg_length_ == socket()->received_;	    
	};

	virtual int MessageSize() {
	    if (socket()->received_ > 0) {
		return socket()->received_;
	    }

	    assert(socket()->recveive_msg_->size > 4);
	    if (received_ >= 4) {
		return socket()->recveive_msg_length_ = 
		    (*(int*)(socket()->recveive_msg_->data));		
	    }
	    return 0;
	};
    };

    class TCPSocket : public EventListener {
    public:
	struct TCPMessage {
	    char *data;
	    int size;
	    int capacity;
	};
    public:
	TCPSocket(int fd) {
	    set_fd(fd);
	};
	TCPSocket();
	~TCPSocket();

	void SetProcessor(Procesor *proc) { 
	    processor_ = proc; 
	};

	void SetMessagizor(Messagizizor *m) { 
	    messagizor_ = m; 
	};

	bool AsyncConnectTo(struct sockaddr_in &addr) {
	    int ret = connect(fd(), (struct sockaddr*)&addr, sizeof(addr));
	    if (ret < 0) {
		return false;
	    }
	    return true;
	};

	int AsyncSend(char *buf, int size, int capacity) {
	    TCPMessage *msg = new TCPMessage;
	    msg->data = buf;
	    msg->size = size;
	    msg->capacity = capacity;
	    
	    send_msg_list_.push_back(msg);
	};

	int Close() { 
	    close(fd());
	    if (msg_) {
		delete[] msg_->data;
		delete msg_;
	    }
	    std::list<TCPMessage*>::iterator it = send_msg_list_.begin();
	    std::list<TCPMessage*>::iterator endit = send_msg_list_.end();
	    for (; it != endit;) {
		delete[] (*it)->data;
		it = send_msg_list_.erase(it);
	    }
	};

	TCPMessage* GetMessage() {
	    if (messagizizor_->IsComplete()) {
		TCPMessage *ret = msg_;
		msg_ = 0;
		received_ = 0;
		return ret;
	    }
	    return 0;
	};

    public:
	int OnWritable() {
	    if (send_msg_list_.size() == 0) {
		return -1;
	    }
	    struct iovec *v = new struct iovec[send_msg_list_.size()];
	    std::list<TCPMessage*>::iterator it = send_msg_list_.begin();
	    std::list<TCPMessage*>::iterator endit = send_msg_list_.end();
	    for (int i = 0; it != endit; it++, i++) {
		if (i > 0) {
		    v[i].iov_base = (*it)->data;
		} else {
		    v[i].iov_base = (*it)->data + sent_;
		}
		v[i].iov_len = (*it)->size;
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

		int len = (*it)->size;
		if (len <= left) {
		    delete[] = (*it)->data;
		    it = send_msg_list_.erase(it);
		    left -= len;
		} else {
		    sent_ = left;
		    break;
		}
	    }

	    delete[] v;
	    return 0;
	};
	// return value:
	// 0: OK and not complete
	// 1: OK and complete
	// <0: error
	int OnReadable() {

	    if (msg_ == 0) {
		assert(received_ == 0);
		msg_ = new TCPMessage;
		msg_->data = malloc(1024);
	    }
	    assert(!messagizizor_->IsComplete());

	    int ret = 0;
	    while (true) {
		int msg_size = messagizizor_->MessageSize();
		int buf_size = msg_->size - received_;
		if (msg_size > 0) {
		    if (msg_size > msg_->size) {
			char *larger_buf = malloc(1024 * 1024);
			memcpy(larger_buf, msg_->data, received_);
			delete[] msg_->data;
			msg_->data = larger_buf;
			buf_size = msg_size - received_;
		    }
		}

		buf = msg_->data + received_;
		int num = read(fd(), buf, buf_size);
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
		received_ += num;
		if (messagizor_->IsComplete()) {
		    ret = 1;
		    break;
		}
	    } // while

	    return ret;
	};

	int OnError() {
	    Close();
	    return 0;
	};

    private:
	friend class Messagizor;
	Messagizizor *messagizizor_;
	Procesor *processor_;
	TCPMessage *msg_;
	int received_;
	std::list<TCPMessage*> send_msg_list_;
	int sent_;
    };

    class TCPListener : public EventListener {
    public:
	TCPListener(){
	};

	~TCPListener(){
	};
	
	void SetProcessor(Procesor *proc) { 
	    processor_ = proc; 
	};
	
	bool ListenOn(struct sockaddr_in &addr) {
	    int fd = socket(PF_INET, SOCK_DATAGRAM, 0);
	    if (fd < 0) {
		return false;
	    }
	    MakeNonBlock(fd);
	    set_fd(fd);

	    if (bind(fd(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		return false;
	    }
	    if (listen(fd(), 1024) < 0) {
		return false;
	    }

	    return true;
	};

    public:
	int Handle() {
	    assert (events() & EPOLLIN);
	    int newsk = OnAcceptable();
	    if (newsk <= 0) {
		return -1;
	    }
	    assert(processor_);
	    int ret = processor_->ProcessNewConnection(newsk);
	    if (ret < 0) {
		return -2;
	    }
	    return 0;
	};

	int OnAcceptable() {
	    struct sockaddr_in addr;
	    socklen_t len;
	    int sk = accept(fd(), (struct sockaddr*)&addr, &len);
	    if (sk < 0) {
		return -1;
	    }
	    return sk;
	};

    private:
	Processor *processor_;
    };

    class UDPSocket : public EventListener {
    public:
	struct UDPMessage {
	    char *data;
	    int size;
	    int capacity;
	    struct sockaddr_in addr;
	};
    public:
	UDPSocket() {	};
	~UDPSocket() {};
	int Close() { 
	    close(fd());
	    if (msg_) {
		delete[] msg_->data;
		delete msg_;
	    }
	    std::list<TCPMessage*>::iterator it = send_msg_list_.begin();
	    std::list<TCPMessage*>::iterator endit = send_msg_list_.end();
	    for (; it != endit;) {
		delete[] (*it)->data;
		it = send_msg_list_.erase(it);
	    }
	};
	
	bool BindOn(struct sockaddr_in &addr) {
	    if (bind(fd(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		return false;
	    }
	    return true;
	};

	bool AsyncSend(struct sockaddr_in &to, char *buf, int size, int capacity) {
	    UDPMessage *msg = new UDPMessage;
	    msg->data = buf;
	    msg->size = size;
	    msg->capacity = capacity;
	    send_msg_list_.push_back(msg);
	    return true;
	};
    public:
	virtual int Handle() {
	    int ret = 0;
	    char *data;
	    int size;
	    if (events() & EPOLLIN) {
		ret = OnReadable(&data, &size);
		if (ret < 0) {
		    return -1;
		}
		assert(processor_);
		ret = processor_->ProcessMessage(data, size);
		if (ret < 0) {
		    return -2;
		}
	    }
	    if (events() & EPOLLOUT) {
		ret = OnWritable();
		if (ret < 0) {
		    return -3;
		}
	    }
	    return 0;
	};

	int OnReadable() {

	    char *data = malloc(1024);
	    struct sockaddr_in addr;
	    memset(&addr, 0, sizeof(addr));
	    socklen_t addrlen = 0;
	    int size = recvfrom(fd(), data, 1024, 0, (struct sockaddr*)&addr, &addrlen);
	    if (size < 0) {
		if (errno != EAGAIN && errno != EINTR) {
		    return -1;
		}
		return 0;
	    }
	    UDPMessage *msg = new UDPMessage;
	    msg->data = data;
	    msg->size = size;
	    msg->capacity = 1024;
	    msg->addr = addr;
	    processor_->ProcessMessage(fd(), data, size, addr);
	    return 1;
	};

	int OnWritable() {
	    if (send_msg_list_.size() == 0) {
		return -1;
	    }
	    struct iovec *v = new struct iovec[send_msg_list_.size()];
	    std::list<UDPMessage*>::iterator it = send_msg_list_.begin();
	    std::list<UDPMessage*>::iterator endit = send_msg_list_.end();
	    for (int i = 0; it != endit; it++, i++) {
		v[i].iov_base = (*it)->data;
		v[i].iov_len = (*it)->size;
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

		int len = (*iut)->size;
		
		if (len <= left) {
		    delete[] = (*it)->data;
		    it = send_msg_list_.erase(it);
		    left -= len;
		} else {
		    assert(false);
		}
	    }
	    delete[] v;
	    return 0;

	};
	int OnError() {
	    Close();
	};
	
    private:
	std::list<UDPMessage*> send_msg_list_;
	Processor *processor_;
    };
};

#endif
