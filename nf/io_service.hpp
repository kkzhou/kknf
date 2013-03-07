#ifndef __IO_SERVICE_HPP__
#define __IO_SERVICE_HPP__


namespace NF {
    
    class IOService {
    public:
	IOService() 
	    :context_by_index_(10000) {
	    ENTERING;
	    all_tcp_sockets_.reserve(100000);
	    LEAVING;
	};

	int Init(int epollnum) {
	    ENTERING;
	    for (int i = 0; i < epollnum; i++) {
		int fd = epoll_create(1024);
		if (fd < 0) {
		    LEAVING2;
		    return -1;
		}
		epoll_fd_.push_back(fd);
	    }
	    LEAVING;
	    return 0;
	};

	bool AddReadListener(EventListener *l, int which_epoll = 0) {

	    ENTERING;
	    assert(which_epoll < epoll_fd_.size());	    
	    struct epoll_event e;
	    memset(&e, sizeof(e), 0);
	    e.events = EPOLLIN;
	    e.data.ptr = l;
	    if (epoll_ctl(epoll_fd_[which_epoll], EPOLL_ADD, l->fd(), &e) < 0) {
		LEAVING2;
		return false;
	    }
	    LEAVING;
	    return true;
	};

	bool AddWriteListener(EventListener *l, int which_epoll = 0) {

	    ENTERING;
	    assert(which_epoll < epoll_fd_.size());	    
	    struct epoll_event e;
	    memset(&e, sizeof(e), 0);
	    e.events = EPOLLOUT;
	    e.data.ptr = l;
	    if (epoll_ctl(epoll_fd_[which_epoll], EPOLL_ADD, l->fd(), &e) < 0) {
		LEAVING2;
		return false;
	    }
	    LEAVING;
	    return true;
	};

	bool AddReadWriteListener(EventListener *l, int which_epoll = 0) {

	    ENTERING;
	    assert(which_epoll < epoll_fd_.size());	    
	    struct epoll_event e;
	    memset(&e, sizeof(e), 0);
	    e.events = EPOLLOUT | EPOLLIN;
	    e.data.ptr = l;
	    if (epoll_ctl(epoll_fd_[which_epoll], EPOLL_ADD, l->fd(), &e) < 0) {
		LEAVING2;
		return false;
	    }
	    LEAVING;
	    return true;
	};


	bool DeleteEventListener(int fd, int which_epoll) {
	    ENTERING;
	    assert(which_epoll < epoll_fd_.size());
	    if (epoll_ctl(epoll_fd_[which_epoll], EPOLL_DEL, fd, 0) < 0) {
		LEAVING2;
		return false;
	    }
	    LEAVING;
	    return true;
	};

	void AddTimer(Timer &timer) {
	    ENTERING;
	    timer_queue_.push(timer);
	    LEAVING;
	};

	void DeleteTimer(int id) {
	    ENTERING;
	    bool id_equal(Timer &t) {		
		return t.id() == id;
	    };
	    
	    timer_queue_.erase(remove_if(timer_queue_.begin(), timer_queue_.end(), id_equal), 
			       timer_queue_.end());
	    LEAVING;
	};

	void HandleTimer(Timer &t, int which_engine) {
	    ENTERING;
	    t.Fire();
	    LEAVING;
	};

	void HandleEvent(EventListener *l, int which_engine) {
	    ENTERING;
	    int ret = l->Handle();
	    if (ret < 0) {
		// delete
	    } else if (ret == 1) {
		// shift engine
		ShiftEngine(l, which_engine);
	    } else {
	    } 
	    LEAVING;
	};

	void HandleError(EventListener *l) {
	    ENTERING;
	    LEAVING;
	};

	void EngineStart(int which_epoll) {
	    ENTERING;
	    assert(which_epoll < epoll_fd_.size());
	    struct epoll_event e[kMaxEpoll];
	    epoll_fd = epoll_fd_[which_epoll];

	    while (!stop_) {
		// memset(&e, 0, sizeof(e[0]) * kMaxEpoll);
		Timer const &t = timer_queue_.top();
		int timeout = 0;
		struct timeval now;
		gettimeofdate(&now, 0);
		timeout = (now.tv_sec - t.fire_time.tv_sec) * 1000;
		timeout += (now.tv_usec - t.fire_time.tv_usec) / 1000;

		int error = 0;

		int ret = epoll_wait(epoll_fd, e, kMaxEpoll, timeout);
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
		    HandleTimer(t, which_epoll);
		    timer_queue_.pop();
		}
		
		for (int i = 0; i < ret; j++) { 
		    EventListener *l = reinterpret_cast<EventListener*>(e[i].data.ptr);
		    HandleEvent(l, which_epoll);
		}     // for i
		
	    } // for (;;)	    
	    LEAVING;
	};

	void EngineStop() { 
	    ENTERING;
	    stop_ = true; 
	    LEAVING;
	};

	struct ThreadArg {
	    int which_engine;
	    IOService *ios;
	};

	static void ThreadProc(void *arg) {
	    ENTERING;
	    ThreadArg *a = reinterpret_cast<ThreadArg*>(arg);
	    int which_engine = a->which_engine;
	    IOService *ios = a->ios;
	    ios->EngineStart(which_engine);
	    LEAVING;
	};

	void RunIOService() {
	    ENTERING;
	    Init();

	    // tcp listener
	    for (int i = 0; i < listener_.size(); i++) {
		AddReadListener(listener_[i], 0);
	    }

	    // udp socket
	    for (int i = 0; i < udp_socket_.size(); i++) {
		AddReadListener(udp_socket_[i], 0);
	    }

	    int thread_num = epoll_fd_.size();
	    pthread_t *pid = new pthread_t[thread_num];

	    for (int i = 0; i < thread_num; i++) {
		ThreadArg *arg = new ThreadArg;
		arg->which_engine = i;
		arg->ios = this;
		int ret = pthread_create(&pid[i], 0, ThreadProc, arg);
	    }

	    for (int i = 0; i < thread_num; i++) {
		while(pthread_join(pid[i], 0) != 0) {
		}
	    }
	    LEAVING;
	};
	
	int AddTCPSocket(TCPSocket *sk) {
	    ENTERING;
	    assert(all_tcp_sockets_[sk->fd()] == 0);
	    all_tcp_socket_[sk->fd()] = sk;
	    LEAVING;
	    return 0;
	};

	TCPSocket* GetTCPSocket(int fd) {
	    ENTERING;
	    LEAVING;
	    return all_tcp_socket_[fd];
	};
	
	void* GetContextByIndex(unsigned long long index) {
	    ENTERING;
	    void *ctx = context_by_index_[index];
	    LEAVING;
	    return ctx;
	};

	void SetContextByIndex(unsigned long long index, void *ctx) {
	    ENTERING;
	    context_by_index_.insert(pair<unsigned long long, void*>(index, ctx));
	    LEAVING;
	};
	
	void* GetContextByFD(int fd) {
	    ENTERING;
	    TCPSocket *sk = all_sockets_[fd];
	    LEAVING;
	    return sk->context();
	};

	void SetContextByFD(int fd, void *ctx) {
	    ENTERING;
	    TCPSocket *sk = all_sockets_[fd];
	    sk->set_context(ctx);
	    LEAVING;
	};

    public:
	int AddTCPListener(char *ipstr, unsigned short hport, Messagizor *m,
			   ListenerProcessor *p) {
	    ENTERING;
	    assert(listener_.size() < kMaxListenerNum);
	    struct sockaddr_in addr;
	    memset(&addr, 0, sizeof(addr0));
	    if (inet_aton(ipstr, &addr.sin_addr) < 0) {
		LEAVING2;
		return -1;
	    }
	    addr.sin_family = AF_INET;
	    addr.sin_port = htons(hport);

	    TCPListener *listener = new TCPListener(m, p);
	    listener->ListenOn(addr);
	    listener_.push_back(listener);
	    LEAVING;
	    return 0;
	};

	int AddUDPSocket(char *ipstr, unsigned short hport, UDPProcessor *p) {

	    ENTERING;
	    if (udp_socket_.size() == kMaxUDPSocketNum) {
		LEAVING2;
		return -1;
	    }

	    struct sockaddr_in addr;
	    memset(&addr, sizeof(addr), 0);
	    if (inet_aton(ipstr, &addr.sin_addr) < 0) {
		LEAVING2;
		return -1;
	    }
	    addr.sin_port = htons(hport);
	    addr.sin_family = AF_INET;
	    udp_socket = new UDPSocket(p);
	    
	    if (!udp_socket->BindOn(addr)) {
		LEAVING2;
		return -2;
	    }
	    udp_socket_.push_back(udp_socket);
	    LEAVING;
	    return 0;
	};
	
	void ShiftEngine(EventListener *l, int cur_engine) {
	    ENTERING;
	    int which = cur_engine++;
	    which = which % epoll_fd_.size();
	    AddReadListener(l, which);
	    LEAVING;
	};

	static void BeepTimerHandler(void *arg) {
	    ENTERING;
	    LEAVING;
	    return;
	};
	// setter&getter
	void set_backendpool(BackendPool *p) {
	    backendpool_ = p;
	};

	BackendPool* backendpool() {
	    return backendpool_;
	};

    private:
	std::vector<TCPSocket*> all_tcp_socket_;
	std::vector<int> listener_;
	std::vector<UDPSocket*> udp_socket_;

	std::vector<int> epoll_fd_;
	bool stop_;
	std::priority_queue<Timer> timer_queue_;

	std::unordered_map<unsigned long long, void*> context_by_index_;

	BackendPool *backendpool_;

	static const int kMaxEpoll = 1000000;
	static const int kMaxUDPSocketNum = 10;
	static const int kMaxListenerNum = 10;
    };
};

#endif
