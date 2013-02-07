#ifndef __SKELETON_HPP__
#define __SKELETON_HPP__
namespace NF {
    class ServerSkeleton {
    public:
	ServerSkeleton();
	void Init();
	
	
	int AddTCPListener(char *ipstr, unsigned short hport);
	int AddUDPSocket(char *ipstr, unsigned short hport);

    private:
	std::vector<Engine*> engines_;
	std::vector<TCPListener*> listeners_;
	UDPSocket *udp_socket_;
    };
};

#endif
