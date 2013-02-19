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
	    ENTERING;
	    LEAVING;
	};

	void Fire() { 
	    ENTERING;
	    handler_(context_); 
	    LEAVING;
	};
	
	bool operator< (Timer &t) {
	    ENTERING;
	    if (fire_time().tv_sec > t->fire_time().tv_sec) {
		LEAVING;
		return true;
	    } else if (fire_time().tv_sec == t->fire_time().tv_sec) {
		if (fire_time().tv_usec > t->fire_time().tv_usec) {
		    LEAVING;
		    return true;
		}
	    } else {
		
	    }
	    LEAVING;
	    return false;

	};
	
	int id() { 
	    ENTERING;
	    LEAVING;
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
