#ifndef __EVENT_ENGINE_HPP__
#define __EVENT_ENGINE_HPP__

#define EVENT_READ 0x00000001U
#define EVENT_WRITE 0x00000002U
#define EVENT_ERROR 0x00000004U

namespace ZXBNF {

    class Event {
    public:
	Event(int id, int fd, void *arg, unsigned int events) 
	    : id_(id), fd_(fd), arg_(arg), events_(events){ };
	~Event() {};
    private:
	int id_;
	int fd_;
	unsigned int events_;
	void *arg_;
	Event *next_;
	Event *prev_;
    private:
	Event() {};
    };

    class Timer {
    public:
	Timer(int usec, bool cycle, int(*handle)(void*), void *arg) 
	    : usec_(usec), 
	      cycle_(cycle),
	      handle_(handle),
	      arg_(arg) {

	    gettimeofday(&next_time_, 0);
	    unsigned long s = (next_time_.tv_usec + usec_) / 1000000;
	    unsigned long u = (next_time_.tv_usec + usec_) % 1000000;

	    next_time_.tv_sec += s;
	    next_time_.tv_usec = u;
	};

	void SetTrigger(int usec = -1) { 

	    assert(!cycle_);
	    if (usec > 0) {
		usec_ = usec; 		
	    }


	    gettimeofday(&next_time_, 0);
	    unsigned long s = (next_time_.tv_usec + usec_) / 1000000;
	    unsigned long u = (next_time_.tv_usec + usec_) % 1000000;

	    next_time_.tv_sec += s;
	    next_time_.tv_usec = u;

	};

	bool operator<(Timer &r) {
	    if (next_time_.sec > r.next_time_.sec) {
		return true;
	    } else if (next_time_.sec == r.next_time_.sec) {
		if (next_time_.usec > r.next_time_.usec) {
		    return true;
		}
	    }
	    return false;
	};

	inline int id() { return id_; };

	~Timer(){};
    private:
	int id_;
	int usec_;
	struct timeval next_time_;
	bool cycle_;
	int (*handle_)(void*);
	void *arg_;
    };

    class EventEngine {
    public:
	EventEngine() : event_list_(0) {};
    public:
	int Run(int msec);
	int AddEvent(Event *e);
	int AddTimer(Timer *e);
    private:
	queue<Timer> timer_queue_;
	Event *event_list_;
    };
};

#endif
