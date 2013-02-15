#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

namespace NF {

    class Timer {
    public:
	typedef (void)(*TimerHandler)(void*);
	Timer(int id, void *context， TimerHandler h, Time &fire_time)
	    : id_(id), fire_time_(fire_time), handler_(h), context_(context) {
	};

	void Fire() { 
	    handler_(context_); 
	};
	
	bool operator< (Timer &t) {
	    if (fire_time().tv_sec > t->fire_time().tv_sec) {
		return true;
	    } else if (fire_time().tv_sec == t->fire_time().tv_sec) {
		if (fire_time().tv_usec > t->fire_time().tv_usec) {
		    return true;
		}
	    } else {
		
	    }
	    return false;

	};
	
	int id() { 
	    return id_; 
	};
    private:
	int id_;
	Time fire_time_;
	TimerHandler handler_;
	void *context_;
    };

    class EventListener {
    public:
	EventListener() {
	    
	};

	virtual ~EventListener() {
	    
	};
	
	virtual int Handle() = 0;

	void set_context(void *ctx) {
	    context_ = ctx;
	};

	void* context() {
	    return context_;
	};

	void MakeNonBlock() {
	    int flags; 
	    if ((flags = fcntl(fd(), F_GETFL, 0)) < 0) { 
		return -1;
	    } 
	    if (fcntl(fd(), F_SETFL, flags | O_NONBLOCK) < 0) { 
		return -2;
	    }
	    return 0;
	};
	
	unsigned int events() { 
	    return events_; 
	};
	
	void set_events(unsigned int events) { 
	    events_ = events; 
	};

	int fd() { 
	    return fd_; 
	};
	
	void set_fd(int fd) { 
	    fd_ = fd; 
	};
	
    private:
	unsigned int events_;
	int fd_;
	void *context_;
    };

    class Engine {
	Engine() {
	};

	~Engine() {
	};

    public:
	bool InitEngine() {
	    epoll_fd_ = epoll_create(kMaxEpoll);
	    if (epoll_fd_ < 0) {
		return false;
	    }
	    return true;
	};

	bool AddEvent(EventListener *l) {
	    struct epoll_event e;
	    memset(&e, sizeof(e), 0);
	    e.events = events;
	    e.data.ptr = l;
	    if (epoll_ctl(epoll_fd_, EPOLL_ADD, l->fd(), &e) < 0) {
		return false;
	    }
	    return true;
	};

	bool DeleteEventListener(int fd) {
	    if (epoll_ctl(epoll_fd_, EPOLL_DEL, fd, 0) < 0) {
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

    private:
	int epoll_fd_;
	bool stop_;
	std::priority_queue<Timer> timer_queue_;
	static const int kMaxEpoll = 1000000;
    };

    class Time {
    public:
	static int TimeToString(time_t time_sec, std::string &time_str) {

	    if (time_sec < 0) {
		return -1;
	    }

	    struct tm *plocaltime = localtime(&time_sec);
	    std::vector<char> buf;
	    buf.resize(100);
	    size_t ret = strftime(&buf[0], buf.size(), "%Y-%m-%d %H:%M:%S", plocaltime);
	    if (ret == 0) {
		return -2;
	    }
	    time_str.assign(&buf[0], ret);
	    return 0;
	};

	static int CurrentTimeString(std::string &time_str) {

	    time_t curtime = time(0);
	    if (curtime == -1) {
		return -1;
	    }
	    int ret = TimeToString(curtime, time_str);
	    return ret;
	};

	static int StringToTime(std::string &time_str, time_t &time_sec) {

	    struct tm local_time;
	    memset(&local_time, 0, sizeof(local_time));
	    size_t ret = sscanf(time_str.c_str(), "%d-%d-%d %d:%d:%d",
				&local_time.tm_year, &local_time.tm_mon, &local_time.tm_mday,
				&local_time.tm_hour, &local_time.tm_min, &local_time.tm_sec);

	    if (ret == 0U || ret != 6U) {
		return -1;
	    }

	    local_time.tm_year -= 1900;
	    local_time.tm_mon -= 1;
	    local_time.tm_isdst = 0;

	    time_sec = mktime(&local_time);
	    return 0;
	};

	inline unsigned long long second() { return second_; };
	inline unsigned long long microsecond() { return microsecond_; };
	inline void set_second(unsigned long long second) { second_ = second; };
	inline void set_microsecond(unsigned long long microsecond) { 
	    microsecond_ = microsecond; 
	};
    private:
	unsigned long long second_;
	unsigned long long microsecond_;
    };

};

#endif
