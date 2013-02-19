#ifndef __MESSAGIZOR_HPP__
#define  __MESSAGIZOR_HPP__

namespace NF {

    class Messagizor {
    public:
	Messagizizor(TCPSocket *sk) : socket_(sk) {
	    ENTERING;
	    LEAVING;
	};
	
	virtual bool IsComplete() = 0;
	virtual int MessageSize() = 0;
	TCPSocket* socket() { 
	    ENTERING;
	    LEAVING;
	    return socket_; 
	};
    private:
	TCPSocket *socket_;
    };

    class LVMessagizor : public Messagizor {
    public:
	LVMessagizor(TCPSocket *sk) : Messagizor(sk) {
	    ENTERING;
	    LEAVING;
	};

	virtual bool IsComplete() {
	    ENTERING;
	    LEAVING;
	    return socket()->receive_msg_length_ == socket()->received_;	    
	};

	virtual int MessageSize() {
	    ENTERING;
	    if (socket()->received_ > 0) {
		LEAVING;
		return socket()->received_;
	    }

	    assert(socket()->recveive_msg_->size > 4);
	    if (received_ >= 4) {
		LEAVING;
		return socket()->recveive_msg_length_ = 
		    (*(int*)(socket()->recveive_msg_->data));		
	    }
	    LEAVING;
	    return 0;
	};
    };
};

#endif
