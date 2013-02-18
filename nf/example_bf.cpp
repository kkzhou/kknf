
#include "skeleton.hpp"

class TestProcessor : public TCPProcessor {
public:
    virtual int ProcessTCPMessage(int fd, char *data, int size) {	   
    };
    virtual int ProcessUDPMessage(int fd, char *data, int size, 
				  struct sockaddr_in &from) {

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

};

class ExampleBF : public Skeleton {
public:
    virtual void Init() {
    };
private:

};

