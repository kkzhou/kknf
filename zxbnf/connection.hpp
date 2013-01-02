
#ifndef __CONNECTION_H__

#include "buffer.hpp"
#include <list>

namespace ZXBNF {

    enum ConnectionType {
	T_SERVER = 1,
	T_CLIENT
    };

    class Connection {
	
    public:
	Connection(Socket *socket) {
	    
	};
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
