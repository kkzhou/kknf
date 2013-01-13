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

    class Event {
    public:
	Event(int id, int fd, void *arg, unsigned int events) 
	    : id_(id), fd_(fd), arg_(arg), events_(events){ };
	~Event() {};

	inline int id() { return id_; };
	inline int fd() { return fd_; };
	intline unsigned int events() { return events_; };

    private:
	int id_;
	int fd_;
	unsigned int events_;
	void *arg_;

    private:
	Event() {};
	Event(Event&){};
	Event& operator=(Event&){};
    };

    struct TimerCompare {
	bool operator()(Timer *t1, Timer *t2) {
	    if (t1->next_time().tv_sec > t2->next_time().tv_sec) {
		return true;
	    } else if (t1->next_time().tv_sec == t2->next_time().tv_sec) {
		if (t1->next_time().tv_usec > t2->next_time().tv_usec) {
		    return true;
		}
	    } else {
		
	    }
	    return false;
	};
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

	inline int id() { return id_; };
	inline struct timeval const &next_time() { return next_time_; };
	int CallHandle() { return handle_(arg); };
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
	int Run(int msec) {
	    
	    int run_out = 0;
	    while (run_out < msec) { // until 'msec' is run out

		// get timeout
		int next_wake = 0;
		struct timeval now;
		gettimeofday(&now, 0);
		Timer *first_timer = timer_queue_.front();
		next_wake = (first_timer->next_time().tv_sec - now.tv_sec) * 1000;
		next_wake += (first_timer->next_time().tv_usec - now.tv_usec) / 1000;
		next_time = next_time <= msec - run_out ? next_time : msec - run_out;

		// add events to epoll
	    }
	};

	int AddEvent(Event *e) {
	    ENTERING;
	    std::map<int, Event*>::iterator it = events_.find(e->id());
	    if (it != events_.end()) {
		LEAVING2;
		return -1;
	    }
	    events_.insert(make_pair<int, Event*>(e->id(), e));
	    LEAVING;
	    return 0;
	};
	void DeleteEvent(int id) {
	    events_.erase(id);
	};
	void AddTimer(Timer &timer) {
	    timer_queue_.push(timer);
	};
    private:
	priority_queue<Timer*, std::vector<Timer*>, TimerCompare> timer_queue_;
	std::map<int, Event*> events_;
    };
};

#endif
