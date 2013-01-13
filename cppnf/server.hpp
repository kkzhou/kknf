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

namespace ZXBNF {
    class Server {
    public:
	Server() 
	    : idle_server_sockets_(kMaxListenSocketNum),
	      idle_client_socket_(KMaxClientSocketNum),
	      udp_server_socket_(-1),
	      udp_client_socket_(-1) {
	    
	};

	~Server(){};

    public:
	int AddTCPLVListenSocket(int index, char *ipstr, unsigned short hport);
	int AddTCPLVClientSocket(int index, char *ipstr, unsigned short hport);

	int AddUDPServerSocket(char *ipstr, unsigned short hport);
	int AddUDPClientSocket(char *ipstr, unsigned short hport);

    public:
	int OnSocketError(int socket);
	int OnSocketReadable(int socket);
	int OnSocketWritable(int socket);
	
    private:
	std::vector<int> listen_sockets_;
	std::vector<std::list<int> > idle_server_sockets_;

	std::vector<std::list<int> > idle_client_sockets_;
	int udp_client_socket_;
	int udp_server_socket_;

	std::vector<AsyncTCPSocket*> all_tcp_sockets_; // indexed by 'fd'

    private:
	EventEngine *engine_;

    private:
	static const int kMaxListenSocketNum = 1000;
	static const int kMaxClientSocketNum = 1000;
    };

};

#endif
