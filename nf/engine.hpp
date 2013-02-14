#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__
namespace NF {


    class Timer {
    public:
	Timer(int id, void *context, Time *fire_time);
	virtual void Fire() = 0;
	bool operator< (Timer &t);
	int id() { return id_; };
    private:
	int id_;
	Time fire_time_;
	void *context_;
    };

    class EventListener {
    public:
	EventListener();
	virtual ~EventListener();
	virtual int Handler() = 0;
	unsigned int events() { return events_; };
	void set_events(unsigned int events) { events_ = events; };
	int fd() { return fd_; };
	void set_fd(int fd) { fd_ = fd; };
    private:
	unsigned int events_;
	int fd_;
    };

    class Engine {
	Engine();
	~Engine();
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
	void Start();
	void Stop();

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