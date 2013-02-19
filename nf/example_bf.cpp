
#include "io_service.hpp"
#include "example_message.hpp"

struct BFContext {
    int user_id;
    int user_salary;
    int user_age;
    int sid;
};

class BFProcessor : public TCPProcessor {
public:
    virtual int ProcessMessage(int fd, char *data, int size) {	   

	TCPSocket *sk = ios()->FindTCPSocket(fd);
	BFContext *ctx = new BFContext;

	ios()->SetContextByFD(sk->fd(), ctx);
	BFReq *req = data + 4;
	int userid = req->user_id;

	BBReq1 *r1 = new BBReq1;
	r1->user_id = userid;
	TCPSocket *bb1 = ios()->GetClient(2);
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

class BF {
public:

private:

};

