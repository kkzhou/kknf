#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "buffer.hpp"
#include "address.hpp"

namespace ZXBNF {

    class Message {
    public:
	Message(Buffer *buffer_list):
	    head_(0),
	    message_size_(-1) {
	    for (Buffer *it = buffer_list; it; it = it->next()) {
		if (it->next()) {
		    assert(it->head() == 0);
		    assert(it->tail() == it->length());
		}
		message_size_ += it->tail();
	    }
	};
	virtual ~TCPMessage() {};

	inline int message_size() { return message_size_; };
	inline Buffer* BufferList() { return head_; };
	inline Buffer* Destroy()  {
	    Buffer *ret = head_;
	    head_ = cur_ = 0;
	    message_size_ = -1;
	    return ret;
	};
    private:
	Buffer *head_;	// Maybe not 'one' buffer but a list
	int message_size_;
	
    private:
	// forbid
	Message(Message&){};
	Message& operator=(Message&){};
    };

    class UDPMessage : public Message {
    public:
	UDPMessage(Buffer *buffer_list, Address &from, Address &to) :
	    Message(buffer_list),
	    from_addr_(from),
	    to_addr_(to) {};

    private:
	Address from_addr_;
	Address to_addr_;
    private:
	UDPMessage(UDPMessage&){};
	UDPMessage& operator=(UDPMessage&){};
    };

    class TCPMessageToSend : public Message {
    public:
	TCPMessageToSend(Buffer *buffer_list):
	    TCPMessage(buffer_list),
	    cur_(0){

	};
	~TCPMessageToSend(){};

	inline bool AllSent() { return cur() == 0; };
	inline int SeekForward(int num) {
	    assert(cur());
	    // check length
	    int left = 0;
	    Buffer *iter = cur();
	    left = num;
	    while (cur() && left > 0) {
		if (cur()->tail() - cur()->head() <= left) {
		    left -= cur()->tail() - cur()->head();
		    cur()->head() = cur()->tail();
		    cur() = cur()->next();
		} else {
		    cur()->head() += left;
		    left = 0;		    
		}
	    }
	    return num - left;
	};
    private:
	Buffer *cur_;
    private:
	// forbid
	TCPMessageToSend(TCPMessageToSend&) {};
	TCPMessageToSend& operator=(TCPMessageToSend&) {};
    };
};

#endif
