#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

namespace NF {

    class TCPSocket : public EventListener {
    public:
	struct TCPMessage {
	    char *data;
	    int size;
	    int capacity;
	};

    public:
	TCPSocket(int fd, Messagizor *m, TCPProcessor *p)
	    : messagizizor_(m),
	      processor_(p) {

	    ENTERING;
	    set_fd(fd);
	    LEAVING;
	};

	TCPSocket(Messagizor *m, TCPProcessor *p)
	    : messagizizor_(m),	      
	      processor_(p) {
	    ENTERING;
	    LEAVING;
	};

	~TCPSocket() {
	    ENTERING;
	    Close();
	    LEAVING;
	};

	bool AsyncConnectTo(struct sockaddr_in &addr) {
	    ENTERING;
	    int ret = connect(fd(), (struct sockaddr*)&addr, sizeof(addr));
	    if (ret < 0) {
		LEAVING2;
		return false;
	    }
	    LEAVING;
	    return true;
	};

	bool AsyncSend(char *buf, int size, int capacity) {

	    ENTERING;
	    TCPMessage *msg = new TCPMessage;
	    msg->data = buf;
	    msg->size = size;
	    msg->capacity = capacity;
	    
	    if (send_msg_list_.size() > kMaxSendList) {
		LEAVING2;
		return false;
	    }
	    send_msg_list_.push_back(msg);
	    LEAVING;
	    return true;
	};

	int Close() { 
	    ENTERING;
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
	    LEAVING2;
	    return 0;
	};

	TCPMessage* GetMessage() {

	    ENTERING;
	    if (messagizizor_->IsComplete()) {
		TCPMessage *ret = msg_;
		msg_ = 0;
		received_ = 0;
		LEAVING;
		return ret;
	    }
	    LEAVING;
	    return 0;
	};
	
	void GetSendList(std::list<TCPMessage*> &msg_list) {
	    ENTERING;
	    send_msg_list_.swap(msg_list);
	    LEAVING;
	};
	    
    public:
	// return value:
	// 0: OK, and this run ends
	// 1: empty
	// -1: error
	int OnWritable() {
	    ENTERING;

	    assert(send_msg_list_.size());

	    struct iovec *v = new struct iovec[send_msg_list_.size()];
	    int error = 0;

	    while (true) {
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
		    if (errno == EINTR) {
			continue;
		    } else if (errno == EAGAIN) {
			error = 0;
			break;
		    } else {
			error = -1;
			break;
		    }
		}
		if (ret == 0) {
		    assert(false);
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
		} // for

		if (send_msg_list_.size() == 0) {
		    error = 1;
		    break;
		}
		
	    } // while
	    delete[] v;
	    LEAVING;
	    return error;
	};

	// return value:
	// 0: OK and not complete
	// 1: OK and complete
	// <0: error
	int OnReadable() {

	    ENTERING;
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
	    LEAVING;
	    return ret;
	};

	int OnError() {
	    ENTERING;
	    Close();
	    LEAVING;
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
	TCPListener(Messagizor *m, ListenerProcessor *p)
	    :processor_(p),
	     messagizizor_(m) {
	    ENTERING;
	    LEAVING;
	};

	~TCPListener(){
	    ENTERING;
	    LEAVING;
	};
		
	bool ListenOn(struct sockaddr_in &addr) {
	    ENTERING;
	    int fd = socket(PF_INET, SOCK_DATAGRAM, 0);
	    if (fd < 0) {
		LEAVING2;
		return false;
	    }
	    MakeNonBlock(fd);
	    set_fd(fd);

	    if (bind(fd(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		LEAVING2;
		return false;
	    }
	    if (listen(fd(), 1024) < 0) {
		LEAVING2;
		return false;
	    }
	    LEAVING;
	    return true;
	};

    public:
	int Handle() {
	    ENTERING;
	    assert (events() & EPOLLIN);
	    while (true) {
		int newsk = OnAcceptable();
		if (newsk < -1) {
		    break;
		}
		assert(processor_);
		int ret = processor_->ProcessNewConnection(newsk);
		if (ret < 0) {
		    LEAVING2;
		    return -2;
		}
	    }
	    LEAVING;
	    return 1;
	};

	int OnAcceptable() {
	    ENTERING;
	    struct sockaddr_in addr;
	    socklen_t len;
	    int sk = accept(fd(), (struct sockaddr*)&addr, &len);
	    if (sk < 0) {
		if (errno == EAGAIN || errno == EINTR) {
		    LEAVING2;
		    return -1;
		}
		LEAVING2;
		return -2;		
	    }
	    LEAVING;
	    return sk;
	};
	Procesor* processor() { 
	    ENTERING;
	    LEAVING;
	    return processor_;
	};

	Messagizor* messagizizor() {
	    ENTERING;
	    LEAVING;
	    return messagizizor_;
	};
    private:
	Processor *processor_;
	Messagizor *messagizor_;
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
	UDPSocket(UDPProcessor *p)
	    : processor_(p) {	
	    ENTERING;
	    LEAVING;
	};
	
	~UDPSocket() {
	    ENTERING;
	    LEAVING;
	};
	
	int Close() { 
	    ENTERING;
	    close(fd());
	    if (msg_) {
		delete[] msg_->data;
		delete msg_;
	    }
	    std::list<UDPMessage*>::iterator it = send_msg_list_.begin();
	    std::list<UDPMessage*>::iterator endit = send_msg_list_.end();
	    for (; it != endit;) {
		delete[] (*it)->data;
		it = send_msg_list_.erase(it);
	    }
	    LEAVING2;
	};
	
	bool BindOn(struct sockaddr_in &addr) {
	    ENTERING;
	    if (bind(fd(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		LEAVING2;
		return false;
	    }
	    LEAVING;
	    return true;
	};

	bool AsyncSend(struct sockaddr_in &to, char *buf, int size, int capacity) {
	    ENTERING;
	    UDPMessage *msg = new UDPMessage;
	    msg->data = buf;
	    msg->size = size;
	    msg->capacity = capacity;
	    send_msg_list_.push_back(msg);
	    LEAVING;
	    return true;
	};
    public:
	virtual int Handle() {
	    ENTERING;
	    int ret = 0;
	    char *data;
	    int size;
	    int error = 0;
	    if (events() & EPOLLIN) {
		while (true) {
		    ret = OnReadable(&data, &size);
		    if (ret == 0) {
			continue;
		    } else if (ret == -1) {
			error = -1;
			break;
		    }
		    assert(processor_);
		    ret = processor_->ProcessMessage(data, size);
		    if (ret < 0) {
			error = -2;
			break;
		    }
		}
		if (error < 0) {
		    Close();
		}
	    }

	    if (events() & EPOLLOUT) {
		ret = OnWritable();
		if (ret < 0) {
		    LEAVING2;
		    return -3;
		}
	    }
	    LEAVING;
	    return 0;
	};

	// return value:
	// 0: error and continue read
	// -1: error
	// 1: success and continue read
	int OnReadable() {
	    ENTERING;
	    char *data = malloc(1024);
	    struct sockaddr_in addr;
	    memset(&addr, 0, sizeof(addr));
	    socklen_t addrlen = 0;
	    int size = recvfrom(fd(), data, 1024, 0, (struct sockaddr*)&addr, &addrlen);
	    if (size < 0) {
		if (errno != EAGAIN && errno != EINTR) {
		    LEAVING2;
		    return -1;
		}
		LEAVING;
		return 0;
	    }
	    UDPMessage *msg = new UDPMessage;
	    msg->data = data;
	    msg->size = size;
	    msg->capacity = 1024;
	    msg->addr = addr;
	    processor_->ProcessMessage(fd(), data, size, addr);
	    LEAVING;
	    return 1;
	};

	int OnWritable() {
	    ENTERING;
	    if (send_msg_list_.size() == 0) {
		LEAVING2;
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
		LEAVING2;
		return -1;
	    } 
	    if (ret == 0) {
		delete[] v;
		LEAVING2;
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
	    LEAVING;
	    return 0;

	};
	int OnError() {
	    ENTERING;
	    Close();
	    LEAVING;
	    return 0;
	};
	
    private:
	std::list<UDPMessage*> send_msg_list_;
	Processor *processor_;
    };
};

#endif
