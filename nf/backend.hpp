#ifndef __BACKEND_HPP__
#define __BACKEND_HPP__

namespace NF {

    class Backend {
    public:
	Backend(int id) : id_(id) {
	    
	};

	TCPSocket* Get() {
	    TCPSocket *ret = client_idle_list_.top();
	    client_idle_list_.pop();
	    return ret;
	};

	void Add(TCPSocket *socket) {
	    client_idle_list_.push_back(socket);
	};

	int id() { 
	    return id_; 
	};
    private:
	int id_;
	std::list<TCPSocket*> client_idle_list_;
    };

    class BackendPool {
    public:
	TCPSocket* GetBackend(int id) {
	    Backend *b = pool_[id];
	    return b->Get();
	};

	void AddBackend(int id, TCPSocket *sk) {
	    Backend *b = pool_[id];
	    return b->Add(sk);
	};
    private:
	std::map<int, Client*> pool_;
    };

};

#endif

