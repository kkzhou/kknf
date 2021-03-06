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
	static int EventCallback_For_ListenSocket(Event *e, void *arg) {
	
	    assert(e);
	    assert(arg);
	    TCPServer *srv = reinterpret_cast<TCPServer*>(arg);

	    assert(!e->is_writable());
	    assert(!e->is_closed());

	    if (e->is_error()) {
		srv->Restart(1000);	// restart in 1 sec
		return -1;
	    }
	    if (!e->is_readable()) {
		return 1;
	    }
	    int listen_fd = e->socket();
	    int count = 100;
	    while (count--) {
		struct sockaddr_in new_addr;
		socklen_t len = 0;
		int newfd = accept(listen_fd, (struct sockaddr*)&new_addr, &len);
		if (newfd < 0) {
		    if (errno == EAGAIN || errno == EINTR) {
			return 1;
		    } else {
			srv->Restart(1000);
			return -2;
		    }
		}
		AsyncTCPDataSocket *new_socket = MakeNewSocket(newfd);
		if (new_socket) {
		    srv->AddServerSocket(new_socket);
		    Event *e = new Event(new_socket->socket(), EventCallback_For_DataSocket, srv);
		    e->RestoreEvents();
		    srv->event_engine()->AddEvent(e);
		} else {
		    assert(false);
		}
	    } // while
	    srv->event_engine()->DeleteEvent(listen_fd);
	    return 1;
	};

	static int EventCallback_For_DataSocket(Event *e, void *arg) {
	    assert(e);
	    assert(arg);

	    TCPServer *srv = reinterpret_cast<TCPServer*>(arg);
	    AsyncTCPDataSocket *sk = srv->GetSocket(e->fd());

	    assert(sk);
	    int ret = 0;
	    if (e->IsReadable()) {
		Buffer *msg = 0;
		int msg_len;
		ret = sk->OnReadable();
		if (ret > 0) {
		    // complete a message reading
		    if (sk->Messagize(&msg, &msg_len) < 0) {
		    
		    }
		    if (srv->ProcessMessage(sk->socket(), msg, msg_len) < 0) {
		    }
		} else if (ret == 0) {
		} else {
		    srv->event_engine()->DeleteEvent(sk->socket());
		    srv->DestroySocket(sk->socket());
		}
	    }
	    if (e->IsWritable()) {
		ret = sk->OnWritable();
		if (ret < 0) {
		    // error
		    srv->event_engine()->DeleteEvent(sk->socket());
		    srv->DestroySocket(sk->socket());
		} else if (ret == 0) {

		} else {
		    srv->event_engine()->DeleteEvent(sk->socket());
		}
	    }

	};
	static int EventCallback_For_Connect(Event *e, void *arg) {
	    assert(e);
	    assert(arg);
	    assert(!e->is_readable());
	
	    TCPServer *srv = reinterpret_cast<TCPServer*>(arg);
	    srv->AddClientSocket(e->fd());

	    srv->event_engine()->DeleteEvent(e->fd());
	    delete e;
	    return 0;

	};

    public:
	static int TimerCallback_For_Sweep(Timer *t, void *arg) {
	    ENTERING;
	    int next = 5000;	// milliseconds
	    LEAVING;
	    return next;

	};
	static int TimerCallback_For_Nothing(Timer *t, void *arg) {
	    ENTERING;
	    int next = 5000;	// milliseconds
	    LEAVING;
	    return next;

	};
    public:
	int AddListener(char *ipstr, unsigned short hport);
	int AddClient(int backend_index, char *ipstr, unsigned short hport);
	void AttatchEngine(EventEngine *eg) { event_engine_ = eg; };
	static int RunInThread(Server *srv) {
	    
	};
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
	int listen_fd_;
	
	std::vector<std::list<int> > idle_client_fd_;
	std::list<int> server_fd_; // only used for sweeping, a little UGLY
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
