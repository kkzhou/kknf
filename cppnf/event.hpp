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

#define EVENT_READ 0x00000001U
#define EVENT_WRITE 0x00000002U
#define EVENT_ERROR 0x00000004U
#define EVENT_CLOSE 0x00000008U


    class Event {
    public:
	// return value:
	// <0: error and delete event
	// 0: ok and delete event
	// 1: ok and continue event
	typedef int (*EventCallback)(Event*, void *);
	Event(int fd, EventCallback cb, void *arg) 
	    : fd_(fd), 
	      events_(0),
	      events_backup_(0),
	      cb_(cb),
	      cb_arg_(arg){};

	~Event() {};

	inline int &fd() { return fd_; };
	inline unsigned int SetReadEvent() { 
	    unsigned int old = events_; 
	    events_ |= EVENT_READ; 
	    return old;
	};
	inline unsigned int SetWriteEvent() { 
	    unsigned int old = events_; 
	    events_ |= EVENT_WRITE; 
	    return old;
	};
	inline unsigned int SetErrorEvent() { 
	    unsigned int old = events_; 
	    events_ |= EVENT_ERROR; 
	    return old;
	};
	inline unsigned int SetCloseEvent() { 
	    unsigned int old = events_; 
	    events_ |= EVENT_CLOSE; 
	    return old;
	};
	inline bool IsReadable() { return events_ & EVENT_READ; };
	inline bool IsWritable() { return events_ & EVENT_WRITE; ;}
	inline bool IsClosed() { return events_ & EVENT_CLOSE; };
	inline bool IsError() { return events_ & EVENT_ERROR; };

	inline unsigned int GetEpollEvents() {
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
	inline void SetEpollEvents(unsigned int events) {
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

	inline void BackupEvents() {
	    events_backup_ = events_;
	};
    
	inline void RestoreEvents() {
	    events_ = events_backup_;
	};
	inline void ClearEvents() {
	    events_ = 0;
	    events_backup_ = 0;
	};
	int Handler() {
	    assert(cb_);
	    assert(cb_arg_);
	    return cb_(this, cb_arg_);
	};

    private:
	int fd_;
	unsigned int events_;
	unsigned int events_backup_;
	EventCallback cb_;
	void *cb_arg_;
	
    private:
	Event() {};
	Event(Event&){};
	Event& operator=(Event&){};
    };
};
#endif
