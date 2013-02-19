#ifndef __MESSAGIZOR_HPP__
#define  __MESSAGIZOR_HPP__

namespace NF {

    class Messagizor {
    public:
	Messagizizor(TCPSocket *sk) : socket_(sk) {
	};
	
	virtual bool IsComplete() = 0;
	virtual int MessageSize() = 0;
	TCPSocket* socket() { 
	    return socket_; 
	};
    private:
	TCPSocket *socket_;
    };

    class LVMessagizor : public Messagizor {
    public:
	LVMessagizor(TCPSocket *sk) : Messagizor(sk) {
	};

	virtual bool IsComplete() {
	    return socket()->receive_msg_length_ == socket()->received_;	    
	};

	virtual int MessageSize() {
	    if (socket()->received_ > 0) {
		return socket()->received_;
	    }

	    assert(socket()->recveive_msg_->size > 4);
	    if (received_ >= 4) {
		return socket()->recveive_msg_length_ = 
		    (*(int*)(socket()->recveive_msg_->data));		
	    }
	    return 0;
	};
    };
};

#endif
