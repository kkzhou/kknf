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
