#ifndef __IO_SERVICE_HPP__
#define __IO_SERVICE_HPP__

#include "engine.hpp"

namespace NF {

    class BasicProcessor : public Processor {
    public:
	void AttachIOService(IOService *ios) {
	    assert(ios_ == 0);
	    ios_ = ios;
	};

    private:
	IOService *ios_;
    };

    class TCPProcessor : public BasicProcessor {
    public:
	virtual int ProcessNewConnection(int listenfd, int newfd) {
	    TCPSocket *newsk = new TCPSocket(newfd);
	    TCPSocket *listensk = ios_->FindTCPSocket(listenfd);
	    ios_->AddTCPSocket(newsk);
	    Engine *e = listensk->engine();
	    newsk->set_engine(e);
	    e->AddEventListener(newsk, EPOLLIN);
	    ios_->ShiftListener(listenfd);
	    return 0;
	};

    };

    class CommandProcessor : public BasicProcessor {
    public:
	virtual int ProcessUDPMessage(int fd, char *data, int size, 
				  struct sockaddr_in &from) {
	    unsigned int cmd = *(unsigned int*)data;

	    switch (cmd) {
	    case 0x01:
	    	// quit
	    	break;
	    case 0x02:
	    	// change listener's engine	    
		int listenfd = *(int*)(data + 4);
		ios_->ShiftListener(listenfd);
		break;
	    case 0x03:
	    	break;
	    case 0x04:
	    	break;
	    default:
	    	break;
	    }
	};
    };	
    
    class IOService {
    public:
	IOService() : all_sockets_(100000, 0){
	    
	};

	virtual void Init() {};

	static void ThreadProc(void *arg) {
	    Engine *e = reinterpret_cast<Engine*>(arg);	    
	    e->Start();
	};

	virtual void RunIOService() {
	    Init();

	    assert(engines_.size() == command_socket_num_);

	    // tcp listener
	    for (int i = 0; i < listener_num_; i++) {
		engines_[0]->AddEventListener(listener_[i]);
	    }

	    // udp socket
	    for (int i = 0; i < udp_socket_num_; i++) {
		engines_[0]->AddEventListener(udp_socket_[i]);
	    }

	    // command socket(udp), one Engine(thread) one socket
	    for (int i = 0; i < command_socket_; i++) {
		engines_[i]->AddEventListener(command_socket_[i]);
	    }

	    int thread_num = engines_.size();
	    pthread_t *pid = new pthread_t[thread_num];

	    for (int i = 0; i < thread_num; i++) {
		int ret = pthread_create(&pid[i], 0, ThreadProc, engines_[i]);
	    }

	    for (int i = 0; i < thread_num; i++) {
		while(pthread_join(pid[i], 0) != 0) {
		}
	    }
	};
	
	void StopIOService() {
	    int thread_num = engines_.size();
	    for (int i = 0; i < thread_num; i++) {
		engines_[i]->Stop();
	    }
	};

	int ShiftListener(int listenfd) {
	    // shift this listener fd to another epoll(that is, another thread
	    char *buf = new char[8]; // len(4) : fd(4)
	    *(int*)buf = 8;
	    *(int*)(buf + 4) = listenfd;
	    
	    command_socket_->AsyncSend(buf, 8);
	    return 0;
	};

	int AddSocket(TCPSocket *sk) {
	    assert(all_sockets_[sk->fd()] == 0);
	    all_sockets_[sk->fd()] = sk;
	    return 0;
	};

	TCPSocket* FindTCPSocket(int fd) {
	    return all_sockets_[fd];
	};
	
	int AddEngine(Engine *engine) {
	    int ret = engines_.size();
	    engines_.push_back(engine);
	    return ret;
	};

	Engine* FindEngine(int index) {
	    Engine *e = engines_[index];
	    return e;
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
	    int cur = command_socket_num_;
	    command_socket_num_++;
	    command_socket_[cur] = new UDPSocket;
	    if (!command_socket_[cur]->BindOn(addr)) {
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
	std::vector<TCPSocket*> all_sockets_;

	TCPListener* listener_[10];
	int listener_num_;

	UDPSocket *command_socket_[10];
	int command_socket_num_;

	UDPSocket *udp_socket_[10];
	int udp_socket_num_;


	std::unordered_map<unsigned long long, void*> context_by_index_;
	static const int kMaxUDPSocketNum = 10;
	static const int kMaxListenerNum = 10;
    };
};

#endif
