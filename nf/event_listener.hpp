#ifndef __EVENT_LISTENER_HPP__
#define __EVENT_LISTENER_HPP__

#include "util.hpp"

namespace NF {

    class EventListener {
    public:
	EventListener() {
	    ENTERING;
	    LEAVING;
	};

	virtual ~EventListener() {
	    ENTERING;
	    LEAVING;
	};
	
	virtual int Handle() = 0;

	void set_context(void *ctx) {
	    ENTERING;
	    context_ = ctx;
	    LEAVING;
	};

	void* context() {
	    ENTERING;
	    LEAVING;
	    return context_;
	};

	void MakeNonBlock() {
	    ENTERING;
	    int flags; 
	    if ((flags = fcntl(fd(), F_GETFL, 0)) < 0) { 
		LEAVING2;
		return -1;
	    } 
	    if (fcntl(fd(), F_SETFL, flags | O_NONBLOCK) < 0) { 
		LEAVING2;
		return -2;
	    }
	    LEAVING;
	    return 0;
	};
	
	unsigned int events() { 
	    ENTERING;
	    LEAVING;
	    return events_; 
	};
	
	void set_events(unsigned int events) { 
	    ENTERING;
	    events_ = events; 
	    LEAVING;
	};

	int fd() { 
	    ENTERING;
	    LEAVING;
	    return fd_; 
	};
	
	void set_fd(int fd) { 
	    ENTERING;
	    fd_ = fd;
	    LEAVING;
	};
	
    private:
	unsigned int events_;
	int fd_;
	void *context_;
    };
};

#endif
