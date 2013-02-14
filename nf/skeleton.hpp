#ifndef __SKELETON_HPP__
#define __SKELETON_HPP__

#include "engine.hpp"

namespace NF {
    class ServerSkeleton : public Processor {
    public:
	ServerSkeleton();
	void Init(int engine_num) {
	    assert(engine_num > 0);
	    for (int i = 0; i < engine_num; i++) {
		Engine *new_engine = new Engine;
		engines_.push_back(new_engine);
	    }	    
	};
	
	int AddTCPListener(char *ipstr, unsigned short hport, Messagizor *messagizor) {
	    
	};
	int AddUDPSocket(char *ipstr, unsigned short hport) {
	    struct sockaddr_in addr;
	    memset(&addr, sizeof(addr), 0);
	    if (inet_aton(ipstr, &addr.sin_addr) < 0) {
		return -1;
	    }
	    addr.sin_port = htons(hport);
	    addr.sin_family = AF_INET;
	    udp_socket_ = new UDPSocket;
	    if (!udp_socket_->BindOn(addr)) {
		return -2;
	    }
	    return 0;
	};
	static void BeepTimerHandler(void *arg) {
	    ENTERING;
	    
	    LEAVING;
	    return;
	};
	int AddTimer(int id, Time &time, Engine::TimerCallback cb, void *arg) {
	    engines_[0]->AddTimer(id, time, cb, arg);
	    return 0;
	};

    private:
	std::vector<Engine*> engines_;
	
	std::vector<TCPListener*> listeners_;
	UDPSocket *udp_socket_;
    };
};

#endif
