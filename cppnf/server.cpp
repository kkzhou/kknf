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

#include "server.hpp"

namespace ZXBNF {
 
    int Server::AddTCPLVListenSocket(int index, char *ipstr, unsigned short hport) {

	assert(index >= 0);
	assert(ipstr);

	Address addr;
	if (addr.SetAddress(ipstr, hport) != 0) {
	    return -1;
	}

	AsyncTCPLVListenSocket *newsk = new AsyncTCPLVListenSocket(addr);
	if (newsk->Listen() < 0) {
	    return -2;
	}
		
	Event *e = new Event(newsk->socket(), EventCallback_For_TCPListenSocket, this);
	event_engine_.AddEvent(e);
	listen_sockets_[index] = newsk->socket();
	if (newsk->socket() >= all_tcp_sockets_.size()) {
	    return -3;
	}
	all_tcp_sockets_[newsk->socket()] = newsk;
	return 0;
    }

    int Server::AddTCPLVClientSocket(int index, char *ipstr, unsigned short hport) {

	assert(index >= 0);
	assert(ipstr);

	Address addr;
	if (addr.SetAddress(ipstr, hport) != 0) {
	    return -1;
	}

	AsyncTCPLVDataSocket *newsk = new AsyncTCPLVDataSocket();
	if (newsk->socket() >= all_tcp_sockets_.size()) {
	    return -2;
	}
	if (newsk->ConnectTo(addr) < 0) {
	    return -3;
	}
		
	Event *e = new Event(newsk->socket(), EventCallback_For_TCPClientSocketConnect, this);
	event_engine_.AddEvent(e);

	all_tcp_sockets_[newsk->socket()] = newsk;
	return 0;
    }

    int Server::AddUDPServerSocket(char *ipstr, unsigned short hport) {
    }
    int Server::AddUDPClientSocket(char *ipstr, unsigned short hport) {
    }
    int Server::OnSocketError(int socket) {
    }
    int Server::OnSocketReadable(int socket) {
    }
    int Server::OnSocketWritable(int socket) {
    }
};
