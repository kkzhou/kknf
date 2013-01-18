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

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include "event_engine.hpp"

namespace ZXBNF {

    class Server {
    public:
	Server() 
	    :idle_client_sockets_(kMaxBackendNum),
	     all_tcp_sockets_(kMaxSocketNum),
	     udp_server_socket_(-1),
	     udp_client_socket_(-1) {
	};

	~Server(){};
    public:
	static int EventCallback_For_TCPListenSocket(Event *e, void *arg);
	static int EventCallback_For_TCPDataSocket(Event *e, void *arg);
	static int EventCallback_For_UDPSocket(Event *e, void *arg);
	static int EventCallback_For_TCPClientSocketConnect(Event *e, void *arg);

    public:
	// static int TimerCallback_For_Sweep(Timer *t, void *arg);
    public:
	virtual int AddTCPListenSocket(char *ipstr, unsigned short hport);
	virtual int AddTCPClientSocket(int index, char *ipstr, unsigned short hport);
	virtual int ProcessMessage(Buffer *buffer, int size);	
	virtual int ProcessMessageUDP(Buffer *buffer, int size);

	int AddUDPServerSocket(char *ipstr, unsigned short hport);
	int AddUDPClientSocket(char *ipstr, unsigned short hport);

    public:
	AsyncTCPDataSocket* GetIdleTCPClientSocket(int index);
	void ReturnTCPClientSocket(int fd);
	AsyncTCPSocket* GetTCPSocket(int fd);
	void DestroyTCPSocket(int fd);
	
    private:
	int listen_socket_;

	std::vector<std::list<int> > idle_client_sockets_;
	std::list<int> idle_server_sockets_; // only used for sweeping, a little UGLY
	int udp_client_socket_;
	int udp_server_socket_;

	std::vector<AsyncTCPSocket*> all_tcp_sockets_; // indexed by 'fd'

    private:
	EventEngine *engine_;

    private:
	static const int kMaxBackendNum = 100;
	static const int kMaxClientSocketNumPerBackend = 100;
	static const int kMaxServerSocketNum = 1000000;
	static const int kMaxSocketNum = kMaxServerSocketNum + kMaxBackendNum * kMaxClientSocketNumPerBackend;
    };

};

#endif
