
#ifndef __CONNECTION_H__

#include "buffer.hpp"
#include "socket.hpp"

#include <list>

namespace ZXBNF {

    class Connection {

	enum ConnectionType {
	    T_TCP_SERVER = 1,
	    T_TCP_CLIENT,
	    T_UDP
	};
	
    public:
	static Connection* MekeTCPConnection(Socket *socket) {
	};
	static Connection* MakeUDPConnection(char *myip, unsigned short myport) {
	};
	static Connection* MakeTCPConnection(char *toip, unsigned short toport) {
	};
    private:
	Connection(
    private:
	std::list<Buffer*> send_buffer_;
	std::list<Buffer*> recv_buffer_;
	Address my_addr_;
	Address peer_addr_;
	ConnectionType type_;

    };

}; // namespace ZXBNF

#define __CONNECTION_H__
#endif
