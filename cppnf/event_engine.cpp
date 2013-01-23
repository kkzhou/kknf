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

#include "event_engine.hpp"

#include <epoll.h>

namespace ZXBNF {

    int EventEngine::Run() {

	struct epoll_event ev[kMaxEpoll];
	int ev_num = 0;
	int error = 0;
	    
    l_repeat:

	// get timeout
	int next_wake = 0;
	struct timeval now;
	gettimeofday(&now, 0);
	Timer *first_timer = timer_queue_.front();
	next_wake = (first_timer->fire_time().tv_sec - now.tv_sec) * 1000;
	next_wake += (first_timer->fire_time().tv_usec - now.tv_usec) / 1000;

	ev_num = kMaxEpoll;
	int ret = epoll_wait(epoll_fd_, &ev, ev_num, next_wake);
	if (ret == 0) {
	    Timer *t = timer_queue_.front();
	    timer_queue_.pop();
	    if (t->Handler() == 0) {
		timer_queue_.push(t);
	    } else {
		delete t;
	    }
	    goto l_repeat;
	}

	if (ret < 0) {
	    if (errno == EAGAIN || errno == EINTR) {
		continue;
	    }
	    error = -1;
	    goto l_exit;
	}
	// process events
	for (int i = 0; i < ev_num; i++) {
	    assert(ev[i].data.ptr);
	    Event *e = reinterpret_cast<Event*>(ev[i].data.ptr);
	    e->set_epoll_events(ev[i].events);
	    if (e->Handler() <= 0) {
		DeleteEvent(e->fd());
		delete e;
	    }
	    // mod epoll
	    ev[i].events = e->get_epoll_events();
	    e->ModEvent(e);
	}
	goto l_repeat;

    l_exit:
	return error;
    };

    int EventEngine::AddEvent(Event *e) {
	
	struct epoll_event ev;
	memset(&ev, sizeof(e), 0);
	ev.events = e->get_epoll_events();
	ev.data.ptr = e;
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, e->fd(), &ev) < 0) {
	    if (errno == EEXIST) {
		return -1;
	    } else {
		return -2;
	    }
	}
	return 0;
    };

    int EventEngine::DeleteEvent(int fd) {
	
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, 0) < 0) {
	    if (errno == ENOENT) {
		return -1;
	    } else {
		return -2;
	    }
	}
	return 0;
    };

    int EventEngine::Init() {

	epoll_fd_ = epoll_create(kMaxEpoll);
	if (epoll_fd_ < 0) {
	    return -1;
	}
	return epoll_fd_;
    }
};
