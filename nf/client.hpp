#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__

namespace NF {

    class Client {
    public:
	Client(int id);
	TCPSocket* GetClient();
	void AddClient(TCPSocket *socket);
    private:
	int id_;
	std::list<TCPSocket*> client_idle_list_;
    };

    class ClientPool {
    public:
	TCPSocket* GetClient(int id);
	void AddClient(int id, TCPSocket *sk);
    private:
	std::map<int, Client*> pool_;
    };

};

#endif

