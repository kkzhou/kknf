#ifndef __TCP_SERVER_HPP__
#define __TCP_SERVER_HPP__


#include "event_engine.hpp"

namespace ZXBNF {

    class TCPServer {
    public:
	TCPServer(EventEngine *eg) 
	    : event_engine_(eg),
	      idle_client_sockets_(kMaxBackendNum),
	      all_tcp_sockets_(kMaxSocketNum) {
	};

	~TCPServer(){};
    public:
	static int EventCallback_For_ListenSocket(Event *e, void *arg);
	static int EventCallback_For_DataSocket(Event *e, void *arg);
	static int EventCallback_For_Connect(Event *e, void *arg);

    public:
	static int TimerCallback_For_Sweep(Timer *t, void *arg);
	static int TimerCallback_For_Nothing(Timer *t, void *arg);
    public:
	virtual int AddListenSocket(char *ipstr, unsigned short hport);
	virtual int AddClientSocket(int backend_index, char *ipstr, unsigned short hport);
	virtual int ProcessMessage(Buffer *buffer, int size);	
    public:
	void AttatchEngine(EventEngine *eg) { event_engine_ = eg; };
    public:
	AsyncTCPDataSocket* GetIdleClientSocket(int backend_index) {
	    assert(backend_index < idle_client_sockets_.size());
	    AsyncTCPDataSocket *ret = idle_client_sockets_[backend_index].front();
	    if (ret) {
		idle_client_sockets_[backend_index].pop();
	    }
	    return ret;
	};

	void ReturnClientSocket(int backend_index, int fd) {
	    assert(backend_index < idle_client_sockets_.size());
	    idle_client_sockets_[backend_index].push_back(fd);
	};
	AsyncTCPSocket* GetSocket(int fd) {
	    assert(fd < all_tcp_sockets_.size());
	    return all_tcp_sockets_[fd];
	};
	void DestroySocket(int fd) {
	    assert(fd < all_tcp_sockets_.size());
	    assert(all_tcp_sockets_[fd]);
	    delete all_tcp_sockets_[fd];
	    all_tcp_sockets_[fd] = 0;
	};
	
    private:
	int listen_socket_;

	std::vector<std::list<int> > idle_client_sockets_;
	std::list<int> server_sockets_; // only used for sweeping, a little UGLY
	std::vector<AsyncTCPSocket*> all_tcp_sockets_; // indexed by 'fd'

    private:
	EventEngine *engine_;

    private:
	static const int kMaxBackendNum = 100;
	static const int kMaxClientSocketNumPerBackend = 100;
	static const int kMaxServerSocketNum = 1000000;
	static const int kMaxSocketNum = kMaxServerSocketNum + kMaxBackendNum * kMaxClientSocketNumPerBackend;
    };


#endif
