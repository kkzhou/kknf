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


#ifndef __EVENT_HPP__
#define __EVENT_HPP__

namespace ZXBNF {

    class Event {
    public:
	typedef int (*EventCallback)(Event*, void *);
	Event(int fd, EventCallback cb, void *arg) 
	    : fd_(fd), 
	      events_(0),
	      cb_(cb),
	      cb_arg(arg){};

	~Event() {};

	inline int &fd() { return fd_; };
	inline unsigned int set_read_event() { 
	    unsigned int old = events_; 
	    events_ |= EVENT_READ; 
	    return old;
	};
	inline unsigned int set_read_event() { 
	    unsigned int old = events_; 
	    events_ |= EVENT_READ; 
	    return old;
	};
	inline unsigned int set_error_event() { 
	    unsigned int old = events_; 
	    events_ |= EVENT_ERROR; 
	    return old;
	};
	inline unsigned int set_close_event() { 
	    unsigned int old = events_; 
	    events_ |= EVENT_CLOSE; 
	    return old;
	};
	inline bool is_readable() { return events_ & EVENT_READ; };
	inline bool is_writable() { return events_ & EVENT_WRITE; ;}
	inline bool is_closed() { return events_ & EVENT_CLOSE; };
	inline bool is_error() { return events_ & EVENT_ERROR; };

	inline unsigned int get_epoll_events() {
	    unsigned int events = 0;

	    if (is_readable()) {
		events |= EPOLLIN;
	    }
	    if (is_writable()) {
		events |= EPOLLOUT;
	    }
	    if (is_closed()) {
		events |= EPOLLHUP;
	    }
	    if (is_error()) {
		evetns |= EPOLLERR;
	    }
	    return events;
	};
	inline void set_epoll_events(unsigned int events) {
	    events_ = 0;
	    if (events & EPOLLIN) {
		set_read_event();
	    }
	    if (events & EPOLLOUT) {
		set_write_event();
	    }
	    if (events & EPOLLERR) {
		set_error_event();
	    }
	    if (events & EPOLLHUP) {
		set_close_event();
	    }
	};
	int Handler() {
	    assert(cb_);
	    assert(cb_arg_);
	    return cb_(this, cb_arg_);
	};

    private:
	int fd_;
	unsigned int events_;
	EventCallback cb_;
	void *cb_arg_;
	
    private:
	Event() {};
	Event(Event&){};
	Event& operator=(Event&){};
    };
};
#endif
