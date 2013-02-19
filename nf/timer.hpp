#ifndef __TIMER_HPP__
#define __TIMER_HPP__

namespace NF {
    class Timer {
    public:
	typedef (void)(*TimerHandler)(void*);

	Timer(int id, void *context， TimerHandler h, Time &fire_time)
	    : id_(id), 
	      fire_time_(fire_time), 
	      handler_(h), 
	      context_(context) {
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
};
#endif
