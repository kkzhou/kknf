#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

namespace NF {

    class Processor {
    public:
	virtual int ProcessNewConnection(int fd) = 0;
	virtual int ProcessMessage(char *data, int size) = 0;
	virtual int ProcessMessage(char *data, int size, struct sockaddr_in from) = 0;
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
	void SetProcessor(Procesor *proc);
	void SetMessagizor(Messagizizor *m);
	bool ConnectTo(struct sockaddr_in &addr);
	int AsyncSend(char *buf, int size);
	int Close();

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
		v[i].iov_base = (*it)->data->start + (*it)->data->head;
		v[i].iov_len = (*it)->data->tail - (*it)->data->head;
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
		    sent_ = left;
		    left = 0;
		}
	    }
	    assert(left == 0);

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
	    close(fd());
	    return 0;
	};
    private:
	friend class Messagizor;
	Messagizizor messagizizor_;
	Procesor processor_;
	TCPMessage *msg_;
	int received_;
	std::list<TCPMessage*> send_msg_list_;
	int sent_;
    };

    class TCPListener : public EventListener {
    public:
	TCPListener(){};
	~TCPListener(){};
	void SetProcessor(Procesor *proc) { processor_ = proc; };
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
	int Handler() {
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
	Processor *processor_;
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

	int OnReadable(char **data, int *len) {
	    *len = 0;
	    *data = malloc(1024);
	    
	};
	int OnWritable();
	int OnError();
	
    private:
	std::list<UDPMessage*> send_msg_list_;
	Processor *processor_;
    };
};

#endif
