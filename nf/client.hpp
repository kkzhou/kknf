#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

namespace NF {

    class Client {
    public:
	Client(int id) : id_(id) {
	    
	};

	TCPSocket* GetClient() {
	    TCPSocket *ret = client_idle_list_.top();
	    client_idle_list_.pop();
	    return ret;
	};

	void AddClient(TCPSocket *socket) {
	    client_idle_list_.push_back(socket);
	};

	int id() { 
	    return id_; 
	};
    private:
	int id_;
	std::list<TCPSocket*> client_idle_list_;
    };

    class ClientPool {
    public:
	TCPSocket* GetClient(int id) {
	    Cient *c = pool_[id];
	    return c->GetClient();
	};

	void AddClient(int id, TCPSocket *sk) {
	    Cient *c = pool_[id];
	    return c->AddClient(sk);
	};
    private:
	std::map<int, Client*> pool_;
    };

};

#endif

