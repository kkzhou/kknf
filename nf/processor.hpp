#ifndef __PROCESSOR_HPP__
#define __PROCESSOR_HPP__

namespace Nf {

    class Processor {
    public:
	Processor(IOService *ios) : ios_(ios) {	};
	virtual int ProcessMessage(int fd, char *data, int size, 
				      struct sockaddr_in &from) = 0;
	virtual int ProcessMessage(int fd, char *data, int size) = 0;
	virtual int ProcessNewConnection(int listenfd, int newfd) = 0;
    private:
	IOService *ios_;
    };

    class TCPProcessor {
    public:

	virtual int ProcessMessage(int fd, char *data, int size) = 0;
	virtual int ProcessMessage(int fd, char *data, int size, 
				   struct sockaddr_in &from) { 
	    assert(false);
	};
	virtual int ProcessNewConnection(int listenfd, int newfd) {
	    TCPSocket *newsk = new TCPSocket(newfd);
	    TCPSocket *listensk = ios_->FindTCPSocket(listenfd);
	    ios_->AddTCPSocket(newsk);
	    Engine *e = listensk->engine();
	    newsk->set_engine(e);
	    e->AddEventListener(newsk, EPOLLIN);
	    return 0;
	};
				      
    public:
	void AttachIOService(IOService *ios) {
	    assert(ios_ == 0);
	    ios_ = ios;
	};

    private:
	IOService *ios_;
    };

    class TCPProcessor {
    public:

	virtual int ProcessTCPMessage(int fd, char *data, int size) = 0;
	virtual int ProcessUDPMessage(int fd, char *data, int size, 
				      struct sockaddr_in &from) = 0;
	virtual int ProcessNewConnection(int listenfd, int newfd) {
	    TCPSocket *newsk = new TCPSocket(newfd);
	    TCPSocket *listensk = ios_->FindTCPSocket(listenfd);
	    ios_->AddTCPSocket(newsk);
	    Engine *e = listensk->engine();
	    newsk->set_engine(e);
	    e->AddEventListener(newsk, EPOLLIN);
	    return 0;
	};
				      
    public:
	void AttachIOService(IOService *ios) {
	    assert(ios_ == 0);
	    ios_ = ios;
	};

    private:
	IOService *ios_;
    };

};

#endif
