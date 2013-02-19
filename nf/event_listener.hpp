#ifndef __EVENT_LISTENER_HPP__
#define __EVENT_LISTENER_HPP__

namespace NF {

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
};

#endif
