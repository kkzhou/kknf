#include "server.hpp"

class UDPChannel1Event : public Event {
public:
    UDPChannel1Event(Server *srv) : srv_(srv) {};
    ~UDPChannel1Event(){};
    virtual int EventCallback() {
	int ret = 0;
	if (is_readable()) {

	}
    };

private:
    Server *srv_;
};

class TCPLVChannel1Event : public Event {
public:
    TCPLVChannel1Event(Server *srv) : srv_(srv) {};
    ~TCPLVChannel1Event(){};
    virtual int EventCallback() {
	int ret = 0;
	if (is_readable()) {

	}
    };

private:
    Server *srv_;
};
class TCPLVListenChannel1Event : public Event {
public:
    TCPLVListenChannel1Event(Server *srv, int index) : srv_(srv) {};
    ~TCPLVListenChannel1Event(){};
    virtual int EventCallback() {
	int ret = 0;
	if (is_readable()) {

	}
    };

private:
    Server *srv_;
};

