 /*
    Copyright (C) <2011>  <ZHOU Xiaobo(zhxb.ustc@gmail.com)>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef __EVENT_ENGINE_HPP__
#define __EVENT_ENGINE_HPP__

#define EVENT_READ 0x00000001U
#define EVENT_WRITE 0x00000002U
#define EVENT_ERROR 0x00000004U

namespace ZXBNF {
    typedef int (*event_callback)(void*);

    class Event {
    public:
	Event(int fd, unsigned int events) 
	    : fd_(fd), 
	      events_(events) {};

	~Event() {};

	inline int fd() { return fd_; };
	inline unsigned int events() { return events_; };
	int Handler() {
	    return EventCallback();
	};
	// return value:
	// -1: error
	// 0: ok, and delete the event
	// 1:  ok
	virtual int EventCallback() = 0;
    private:
	int fd_;
	unsigned int events_;
	
    private:
	Event() {};
	Event(Event&){};
	Event& operator=(Event&){};
    };

    struct TimerCompare {
	bool operator()(Timer *t1, Timer *t2) {
	    if (t1->fire_time().tv_sec > t2->fire_time().tv_sec) {
		return true;
	    } else if (t1->fire_time().tv_sec == t2->fire_time().tv_sec) {
		if (t1->fire_time().tv_usec > t2->fire_time().tv_usec) {
		    return true;
		}
	    } else {
		
	    }
	    return false;
	};
    };

    class Timer {
    public:
	Timer() {};
	virtual ~Timer(){};
	inline int id() { return id_; };
	inline struct timeval const &fire_time() { return fire_time_; };
	void SetTimer(int msec) {

	    assert(msec > 0);
	    gettimeofday(&fire_time_, 0);
	    
	    unsigned long s = (fire_time_.tv_usec + msec) / 1000000;
	    unsigned long u = (fire_time_.tv_usec + msec) % 1000000;

	    fire_time_.tv_sec += s;
	    fire_time_.tv_usec = u;

	};

	// return value:
	// -2: error when handling, delete the timer
	// -1: time is not up
	// 0: contiue
	// 1: not any more
	int Handler() {
	    struct timeval now;
	    gettimeofday(&now, 0);
	    if (next_time_.tv_sec > now.tv_sec) {
		return -1;
	    }
	    if (next_time_.tv_sec == now.tv_sec) {
		if (next_time_.tv_usec - now.tv_usec > 10000) {
		    return -1;
		}
	    }
	    int msec = TimerCallback();
	    if (msec < 0) {
		return -2;
	    }
	    if (msec == 0) {
		return 1;
	    }
	    SetTimer(msec);
	    return 0;
	};

	// return value:
	// -1: error
	// 0: ok
	// >0: in how many milliseconds trigger next
	virtual int TimerCallback() = 0;

    private:
	int id_;
	struct timeval fire_time_;
    };

    class EventEngine {
    public:
	EventEngine() : epoll_fd_(0), timer_queue_(100) {};

    public:
	int Init();
	int Run();
	int AddEvent(Event *e);
	void DeleteEvent(int fd);

	inline void AddTimer(Timer *timer) {
	    timer_queue_.push(timer);
	};
	inline Timer* DeleteTimer(int id) {
	    Timer *ret = timer_queue_.front();
	    timer_queue_.pop();
	    return ret;
	};
    private:
	priority_queue<Timer*, std::vector<Timer*>, TimerCompare> timer_queue_;
	int epoll_fd_;
	static const int kMaxEpoll = 1024 * 1024;
    };
};

#endif
