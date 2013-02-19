#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

namespace NF {

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

	bool AddEventListener(EventListener *l, unsigned int events) {
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


};

#endif
