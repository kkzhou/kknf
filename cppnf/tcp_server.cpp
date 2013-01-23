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

#include "tcp_server.hpp"

namespace ZXBNF {
    
    int TCPServer::EventCallback_For_ListenSocket(Event *e, void *arg) {
	
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
	while (true) {
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
	return 1;
    };
    int TCPServer::EventCallback_For_DataSocket(Event *e, void *arg) {
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
    int TCPServer::EventCallback_For_Connect(Event *e, void *arg) {
	assert(e);
	assert(arg);
	assert(!e->is_readable());
	
	TCPServer *srv = reinterpret_cast<TCPServer*>(arg);
	srv->AddClientSocket(e->fd());

	srv->event_engine()->DeleteEvent(e->fd());
	delete e;
	return 0;
    };

    int TCPServer::TimerCallback_For_Sweep(Timer *t, void *arg) {
	int next = 1000;	// 1 sec
	
    };
    int TCPServer::TimerCallback_For_Nothing(Timer *t, void *arg) {
	ENTERING;
	int next = 5000;	// milliseconds
	LEAVING;
	return next;
    };

};
