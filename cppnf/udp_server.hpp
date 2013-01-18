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

#ifndef __UDP_SERVER_HPP__
#define __UDP_SERVER_HPP__

#include "event_engine.hpp"

namespace ZXBNF {

    class UDPServer {
    public:
	UDPServer() 
	     udp_server_socket_(-1),
	     udp_client_socket_(-1) {
	};

	~UDPServer(){};
    public:
	static int EventCallback_For_UDPSocket(Event *e, void *arg);
    public:
	static int TimerCallback_For_Nothing(Timer *t, void *arg);
    public:
	virtual int ProcessMessage(Buffer *buffer, int size);	

	int AddUDPServerSocket(char *ipstr, unsigned short hport);
	int AddUDPClientSocket(char *ipstr, unsigned short hport);

    private:
	int udp_client_socket_;
	int udp_server_socket_;
    private:
	EventEngine *engine_;
    };

};

#endif
