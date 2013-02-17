#ifndef __SKELETON_HPP__
#define __SKELETON_HPP__

#include "engine.hpp"

namespace NF {

    class BasicProcessor : public Processor {
    public:
	void AttachIOService(IOService *ios) {
	    assert(ios_ == 0);
	    ios_ = ios;
	};

	virtual int ProcessNewConnection(int fd) {
	    TCPSocket *newsk = new TCPSocket(fd);
	    ios_->AddTCPSocket(newsk);
	    Engine *e = ios_->GetEngine(fd % ios_->GetEngineNum());	  
	    e->AddEventListener(newsk, EPOLLIN);
	    return 0;
	};

	virtual int ProcessMessage(int fd, char *data, int size) {	   
	    return OnRequest(fd, data, size);
	};

	virtual int ProcessMessage(int fd, char *data, int size, struct sockaddr_in &from) {
	    if (fd == command_socket_->fd()) {
		return OnCommand(*(unsigned int*)data, data + 4, size - 4);
	    }
	    
	    return OnRequest(fd, data, size, from);
	};
	
	virtual int OnCommand(unsigned int cmd, char *data, int size) {
	    switch (cmd) {
	    case 0x01:
		// quit
		break;
	    case 0x02:
		// change listener's engine
		int listener = *(int*)data;
		int index = 0;
		for (index = 0; index < kMaxListenerNum; index++) {
		    if (listener_[index] == listener) {
			break;
		    }
		}
		listener_in_which_engine_[index] = 
		    (listener_in_which_engine_[index] + 1) % listener_num_;

		Engine *e = ios_->GetEngine(listener_in_which_engine_[index]);
		e->AddEventListener(all_sockets_[listener], EPOLLIN);
		break;
	    case 0x03:
		break;
	    case 0x04:
		break;
	    default:
		break;
	    }
	};

	virtual int OnRequest(int fd, char *data, int size) {
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
	    // 	TCPSocket *backend1 = ClientPool.GetClient(2); // or, just send throught udp
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
  
	};
	virtual int OnRequest(int fd, char *data, int size, struct sockaddr_in &from) {
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
	    // 	TCPSocket *backend1 = ClientPool.GetClient(2); // or, just send throught udp
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
	};
	
    private:
	IOService *ios_;
    };

    class IOService {
    public:
	IOService() : all_sockets_(100000, -1){
	    
	};

	virtual void Init() = 0;
	virtual void RunForever() {
	    
	};

	int AddSocket(TCPSocket *sk) {
	    assert(all_sockets_[sk->fd()] == -1);
	    all_sockets_[sk->fd()] = sk;
	    return 0;
	};

	Engine* GetEngine(int index) {
	    return engines_[index];
	};

	int GetEngineNum() {
	    return engines_.size();
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
	int AddTCPListener(char *ipstr, unsigned short hport, Messagizor *messagizor) {
	    assert(listener_num_ < kMaxListenerNum);
	    struct sockaddr_in addr;
	    memset(&addr, 0, sizeof(addr0));
	    if (inet_aton(ipstr, &addr.sin_addr) < 0) {
		return -1;
	    }
	    addr.sin_family = AF_INET;
	    addr.sin_port = htons(hport);
	    int cur = listener_num_;
	    listener_num_++;
	    listener_[cur] = new TCPListener;
	    listener_[cur]->SetProcessor(this);
	    listener_[cur]->ListenOn(addr);
	    listener_[cur]->SetMessagizor(new LVMessagizor(listener_[cur]);
	    return 0;
	};

	int AddUDPSocket(char *ipstr, unsigned short hport) {

	    if (udp_socket_num_ == kMaxUDPSocketNum) {
		return -1;
	    }

	    struct sockaddr_in addr;
	    memset(&addr, sizeof(addr), 0);
	    if (inet_aton(ipstr, &addr.sin_addr) < 0) {
		return -1;
	    }
	    addr.sin_port = htons(hport);
	    addr.sin_family = AF_INET;
	    int cur = udp_socket_num_;
	    udp_socket_num_++;
	    udp_socket_[cur] = new UDPSocket;
	    if (!udp_socket_[cur]->BindOn(addr)) {
		return -2;
	    }
	    return 0;
	};
	int AddCommandSocket(char *ipstr, unsigned short hport) {

	    assert(command_socket_ == 0);
	    struct sockaddr_in addr;
	    memset(&addr, sizeof(addr), 0);
	    if (inet_aton(ipstr, &addr.sin_addr) < 0) {
		return -1;
	    }
	    addr.sin_port = htons(hport);
	    addr.sin_family = AF_INET;
	    command_socket_ = new UDPSocket;
	    if (!command_socket_->BindOn(addr)) {
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
	TCPListener* listener_[10];
	int listener_in_which_engine_[10];
	int listener_num_;
	UDPSocket *udp_socket_[10];
	int udp_socket_num_;
	UDPSocket *command_socket_;

	std::unordered_map<unsigned long long, void*> context_by_index_;
	static const int kMaxUDPSocketNum = 10;
	static const int kMaxListenerNum = 10;
    };
};

#endif
