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
	inline EventEngine* event_engin() { return event_engine_; };
    public:
	// return value:
	// <0: error and delete event
	// 0: ok and delete event
	// 1: ok and continue event
	static int EventCallback_For_ListenSocket(Event *e, void *arg);
	static int EventCallback_For_DataSocket(Event *e, void *arg);
	static int EventCallback_For_Connect(Event *e, void *arg);

    public:
	static int TimerCallback_For_Sweep(Timer *t, void *arg);
	static int TimerCallback_For_Nothing(Timer *t, void *arg);
    public:
	int AddListener(char *ipstr, unsigned short hport);
	int AddClient(int backend_index, char *ipstr, unsigned short hport);
	void AttatchEngine(EventEngine *eg) { event_engine_ = eg; };

    private:
	virtual int ProcessMessage(int socket, Buffer *msg, int msg_len) = 0;
	virtual int AddListenSocket(int socket) = 0;
	virtual int AddClientSocket(int socket) = 0;	
    private:
	int AddServerSocket(AsyncTCPDataSocket *sk) {
	    if (sk->socket() >= all_tcp_sockets_.size()) {
		return -1;
	    }
	    server_sockets_.push_back(sk);
	    all_tcp_sockets_[sk->socket()] = sk;
	    return 0;	    
	};

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
