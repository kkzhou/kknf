#ifndef __IO_SERVICE_HPP__
#define __IO_SERVICE_HPP__


namespace NF {
    
    class IOService {
    public:
	IOService() 
	    :context_by_index_(10000) {
	    all_tcp_sockets_.reserve(100000);
	};

	virtual int Init(int threadnum) {
	    for (int i = 0; i < threadnum; i++) {
		int fd = epoll_create(1024);
		if (fd < 0) {
		    return -1;
		}
		epoll_fd_.push_back(fd);
	    }
	    return 0;
	};

	bool AddEventListener(EventListener *l, unsigned int events, int which_epoll = 0) {

	    assert(which_epoll < epoll_fd_.size());
	    struct epoll_event e;
	    memset(&e, sizeof(e), 0);
	    e.events = events;
	    e.data.ptr = l;
	    if (epoll_ctl(epoll_fd_[which_epoll], EPOLL_ADD, l->fd(), &e) < 0) {
		return false;
	    }
	    return true;
	};

	bool DeleteEventListener(int fd, int which_epoll) {
	    assert(which_epoll < epoll_fd_.size());
	    if (epoll_ctl(epoll_fd_[which_epoll], EPOLL_DEL, fd, 0) < 0) {
		return false;
	    }
	    return true;
	};

	void AddTimer(Timer &timer) {
	    timer_queue_.push(timer);
	};

	void DeleteTimer(int id) {
	    bool id_equal(Timer &t) {
		return t.id() == id;
	    };
	    timer_queue_.erase(remove_if(timer_queue_.begin(), timer_queue_.end(), id_equal), 
			       timer_queue_.end());
	};

	void Start() {
	    struct epoll_event e[kMaxEpoll];
	    for (;;) {
		// memset(&e, 0, sizeof(e[0]) * kMaxEpoll);
		Timer const &t = timer_queue_.top();
		int timeout = 0;
		struct timeval now;
		gettimeofdate(&now, 0);
		timeout = (now.tv_sec - t.fire_time.tv_sec) * 1000;
		timeout += (now.tv_usec - t.fire_time.tv_usec) / 1000;

		int error = 0;

		int ret = epoll_wait(epoll_fd_, e, kMaxEpoll, timeout);
		if (ret < 0) {
		    if (errno != EAGAIN && errno != EINTR) {
			error = -1;
			break;
		    }
		    continue;
		} 
		// timeout
		timeout = 0;			
		gettimeofdate(&now, 0);
		timeout = (now.tv_sec - t.fire_time.tv_sec) * 1000;
		timeout += (now.tv_usec - t.fire_time.tv_usec) / 1000;
		if (timeout < 10) {
		    t.Fire();
		    timer_queue_.pop();
		}
		
		for (int i = 0; i < ret; j++) { 
		    EventListener *l = reinterpret_cast<EventListener*>(e[i].data.ptr);
		    if (l->Handler() < 0) {
			DeleteEventListener(l->fd());
		    }
		}     // for i
		
	    } // for (;;)	    
	};

	void Stop() { 
	    stop_ = true; 
	};

	static void ThreadProc(void *arg) {
	    Engine *e = reinterpret_cast<Engine*>(arg);	    
	    e->Start();
	};

	virtual void RunIOService() {
	    Init();

	    assert(engines_.size() == command_socket_num_);

	    // tcp listener
	    for (int i = 0; i < listener_num_; i++) {
		engines_[0]->AddEventListener(listener_[i]);
	    }

	    // udp socket
	    for (int i = 0; i < udp_socket_num_; i++) {
		engines_[0]->AddEventListener(udp_socket_[i]);
	    }

	    // command socket(udp), one Engine(thread) one socket
	    for (int i = 0; i < command_socket_; i++) {
		engines_[i]->AddEventListener(command_socket_[i]);
	    }

	    int thread_num = engines_.size();
	    pthread_t *pid = new pthread_t[thread_num];

	    for (int i = 0; i < thread_num; i++) {
		int ret = pthread_create(&pid[i], 0, ThreadProc, engines_[i]);
	    }

	    for (int i = 0; i < thread_num; i++) {
		while(pthread_join(pid[i], 0) != 0) {
		}
	    }
	};
	
	void StopIOService() {
	    int thread_num = engines_.size();
	    for (int i = 0; i < thread_num; i++) {
		engines_[i]->Stop();
	    }
	};

	int ShiftListener(int listenfd) {
	    // shift this listener fd to another epoll(that is, another thread
	    char *buf = new char[8]; // len(4) : fd(4)
	    *(int*)buf = 8;
	    *(int*)(buf + 4) = listenfd;
	    
	    command_socket_->AsyncSend(buf, 8);
	    return 0;
	};

	int AddSocket(TCPSocket *sk) {
	    assert(all_sockets_[sk->fd()] == 0);
	    all_sockets_[sk->fd()] = sk;
	    return 0;
	};

	TCPSocket* FindTCPSocket(int fd) {
	    return all_sockets_[fd];
	};
	
	int AddEngine(Engine *engine) {
	    int ret = engines_.size();
	    engines_.push_back(engine);
	    return ret;
	};

	Engine* FindEngine(int index) {
	    Engine *e = engines_[index];
	    return e;
	};

	void* GetContextByIndex(unsigned long long index) {
	    void *ctx = context_by_index_[index];
	    return ctx;
	};

	void SetContextByIndex(unsigned long long index, void *ctx) {
	    context_by_index_.insert(pair<unsigned long long, void*>(index, ctx));
	};
	
	void* GetContextByFD(int fd) {
	    TCPSocket *sk = all_sockets_[fd];
	    return sk->context();
	};

	void SetContextByFD(int fd, void *ctx) {
	    TCPSocket *sk = all_sockets_[fd];
	    sk->set_context(ctx);
	};

    public:
	int AddTCPListener(char *ipstr, unsigned short hport, Messagizor *messagizor) {
	    assert(listener_num_ < kMaxListenerNum);
	    struct sockaddr_in addr;
	    memset(&addr, 0, sizeof(addr0));
	    if (inet_aton(ipstr, &addr.sin_addr) < 0) {
		return -1;
	    }
	    addr.sin_family = AF_INET;
	    addr.sin_port = htons(hport);
	    int cur = listener_num_;
	    listener_num_++;
	    listener_[cur] = new TCPListener;
	    listener_[cur]->SetProcessor(this);
	    listener_[cur]->ListenOn(addr);
	    listener_[cur]->SetMessagizor(new LVMessagizor(listener_[cur]);
	    return 0;
	};

	int AddUDPSocket(char *ipstr, unsigned short hport) {

	    if (udp_socket_num_ == kMaxUDPSocketNum) {
		return -1;
	    }

	    struct sockaddr_in addr;
	    memset(&addr, sizeof(addr), 0);
	    if (inet_aton(ipstr, &addr.sin_addr) < 0) {
		return -1;
	    }
	    addr.sin_port = htons(hport);
	    addr.sin_family = AF_INET;
	    int cur = udp_socket_num_;
	    udp_socket_num_++;
	    udp_socket_[cur] = new UDPSocket;
	    if (!udp_socket_[cur]->BindOn(addr)) {
		return -2;
	    }
	    return 0;
	};

	static void BeepTimerHandler(void *arg) {
	    ENTERING;
	    LEAVING;
	    return;
	};

    private:
	std::vector<TCPSocket*> all_tcp_socket_;
	std::vector<int> listener_;
	std::vector<UDPSocket*> udp_socket_;

	std::vector<int> epoll_fd_;
	bool stop_;
	std::priority_queue<Timer> timer_queue_;

	std::unordered_map<unsigned long long, void*> context_by_index_;

	static const int kMaxEpoll = 1000000;
	static const int kMaxUDPSocketNum = 10;
	static const int kMaxListenerNum = 10;
    };
};

#endif
