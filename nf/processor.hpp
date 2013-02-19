#ifndef __PROCESSOR_HPP__
#define __PROCESSOR_HPP__

namespace Nf {

    class TCPProcessor {
    public:
	Processor(IOService *ios) : ios_(ios) {	};
	virtual int ProcessMessage(int fd, char *data, int size) = 0;
    private:
	IOService *ios_;
    };

    class UDPProcessor {
    public:
	UDPProcessor(IOService *ios) : ios_(ios) {	};
	virtual int ProcessMessage(int fd, char *data, int size, 
				   struct sockaddr_in &from) = 0;
    private:
	IOService *ios_;
    };

    class ListenerProcessor {
    public:
	ListenerProcessor(IOService *ios) : ios_(ios) {};
	virtual int ProcessNewConnection(int listenfd, int newfd) {
	    TCPListener *l = ios_->GetTCPSocket(listenfd);
	    TCPSocket *newsk = new TCPSocket(l->messagizizor(), l->processor());
	    ios_->AddTCPSocket(newsk);
	    ios_->AddEventListener(newsk, EPOLLIN);
	    return 0;
	};

    private:
	IOService *ios_;
    };

};

#endif
