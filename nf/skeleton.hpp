#ifndef __SKELETON_HPP__
#define __SKELETON_HPP__

#include "engine.hpp"

namespace NF {

    class ServerSkeleton : public Processor {
    public:
	ServerSkeleton() : all_sockets_(100000, -1){
	    
	};

	void Init() {	    
	};

	void AddEngine(Engine *engine) {
	    engines_.push_back(engine);
	};

	void* GetContextByIndex(unsigned long long index) {
	    void *ctx = context_by_index_[index];
	    return ctx;
	};

	void SetContextByIndex(unsigned long long index, void *ctx) {
	    context_by_index_.insert(pair<unsigned long long, void*>(index, ctx));
	};
	
	void* GetContextByFD(int fd) {
	    TCPSocket *sk = all_sockets_[fd];
	    return sk->context();
	};

	void SetContextByFD(int fd, void *ctx) {
	    TCPSocket *sk = all_sockets_[fd];
	    sk->set_context(ctx);
	};

    public:
	// from interface Processor
	virtual int ProcessNewConnection(int fd) {
	    TCPSocket *newsk = new TCPSocket(fd);
	    assert(all_sockets_[fd] == -1);
	    all_sockets_[fd] = newsk;
	    engines_[fd % engines_.size()]->AddEvent(newsk);
	    return 0;
	};

	virtual int ProcessMessage(int fd, char *data, int size) {
	    // example code
	    // TCPSocket *sk = all_sockets_[fd];
	    // MyContext *ctx = GetContextByFD(sk->fd()); // or GetContextByFD()
	    // if (!ctx) {
	    // 	ctx = new MyContext;
	    // 	SetContextByFD(sk->fd(), ctx);  // or use SetContextByIndex()
	    // }

	    // if (ctx->state == S_INIT) {
	    //     MyBackendRequest1 req1;
	    // 	req1.field1 = 1;
	    // 	req1.field2 = 2;
	    // 	char *req1_buf = new char[req1.size()];
	    // 	req1.Serialize(req1_buf);
	    // 	TCPSocket *backend1 = ClientPool.GetClient(2);
	    // 	backend1->set_context(ctx);		
	    // 	backend1->AsyncSend(req1...);
	    // 	MyBackendRequest2 req2;
	    // 	//...
	    // 	TCPSocket *backend2 = ClientPool.GetClient(4);
	    // 	backend2->set_context(ctx);
	    // 	backend2->AsyncSend(req2...);
	    // 	ctx->state = S_ALL_BACKEND_REQ_SENT;

	    // } else if (ctx->state == S_ALL_BACKEND_REQ_SENT) {
		
	    // } else if (ctx->state == S_BACKEND_RSP1_RECVED) {
	    // } else if (ctx->state == S_BACKEND_RSP2_RECVED) {
	    // } else {
	    // }
	    return 0;
	};

	virtual int ProcessMessage(int fd, char *data, int size, struct sockaddr_in from) {
	};

    public:
	int SetTCPListener(char *ipstr, unsigned short hport, Messagizor *messagizor) {
	    struct sockaddr_in addr;
	    memset(&addr, 0, sizeof(addr0));
	    if (inet_aton(ipstr, &addr.sin_addr) < 0) {
		return -1;
	    }
	    addr.sin_family = AF_INET;
	    addr.sin_port = htons(hport);
	    listener_ = new TCPListener;
	    listener_->SetProcessor(this);

	    listener_->ListenOn(addr);
	    
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


    private:
	std::vector<Engine*> engines_;
	std::vector<int> all_sockets_;
	TCPListener* listener_;
	UDPSocket *udp_socket_;
	std::unordered_map<unsigned long long, void*> context_by_index_;
    };
};

#endif
