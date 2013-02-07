#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__
namespace NF {
    class Timer {
    public:
	Timer(int id, EngineCallback cb, void *arg);
	void Fire();
	bool operator< (Timer &t);
    private:
	int id_;
	struct timeval fire_time_;
	EngineCallback cb_;
	void *arg_;
    };
    class Engine {
    public:
	// return value:
	// -1: error
	// 0: OK and delete
	// 1: OK and continue
	typedef (int)(*EngineCallback)(void*);
    public:
	void AddEventListener(int fd, unsigned short events, EngineCallback cb, void *arg);
	void DeleteEventListener(int fd);
	void AddTimer(int id, struct timeval time, EngineCallback cb, void *arg);
	void DeleteTimer(int id);
	void Start();
	void Stop();
    private:
	int epoll_fd_;
	bool stop_;
	std::priority_queue<Timer> timer_queue_;
    };
};

#endif
