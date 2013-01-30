 /*
    Copyright (C) <2011>  <ZHOU Xiaobo(zhxb.ustc@gmail.com)>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/


#ifndef __ASYNC_UDP_SOCKET_HPP__
#define __ASYNC_UDP_SOCKET_HPP__


namespace ZXBNF {

    class UDPSocket : public AsyncSocket {
    public:
	struct UDPMessage {
	    Buffer *data_;
	    Address addr_;
	};
    public:
	int BindOn(Address &addr) {
	    int fd = socket(PF_INET, SOCK_DATAGRAM, 0);
	    if (fd < 0) {
		return -1;
	    }
	    set_fd(fd);
	    if(bind(fd, (struct sockaddr*)&addr.addr(), addr.addr_len()) < 0) {
		return -2;
	    }
	    return 0;
	};
	int OnReadable() {
	    assert(msg_in_recv_ == 0);

	    Buffer *buf = mempool()->GetSmallBlock();
	    if (!buf) {
		return -1;
	    }
	    Address addr;
	    int num = readfrom(fd(), buf->start, buf->length, 0, 
			       (struct sockaddr*)&addr.addr(), &addr.addrlen());
	    if (num > 0) {
		buf->tail() += num;
	    } else if (num < 0) {
		if (errno != EAGAIN || errno != EINTR) {
		    return -2;
		} else {
		    return 0;
		}
	    } else {
		return 0;
	    }
	    msg_in_recv_ = new UDPMessage;
	    msg_in_recv_->data_ = buf;
	    msg_in_recv_->addr_ = addr;
	    return 1;
	}
	int OnWritable() {

	    UDPMessage *msg = send_msg_list_.front();

	    if (!msg) {
		return -1;
	    }
	    send_msg_list_.pop();
	    Buffer *buf = msg->data_;
	    assert(buf->head == 0);
	    
	    int num = sendto(fd(), buf->start, buf->tail, 0, 
			       (struct sockaddr*)&msg->addr_.addr(), msg->addr_.addrlen());
	    if (num > 0) {
		delete msg;
	    } else if (num < 0) {
		if (errno != EAGAIN || errno != EINTR) {
		    return -2;
		} else {
		    return 0;
		}
	    } else {
		return 0;
	    }
	    if (send_msg_list_.size() == 0) {
		return 1;
	    }
	    
	    return 0;
	};
    public:
	int SendMessage(UDPMessage *msg) {
	    assert(msg);
	    if (send_msg_list_.size() < kMaxUDPSendMessageNum) {
		send_msg_list_.push_back(msg);
		return 0;
	    }
	    return -1;
	};

	int Messagize(UDPMessage **msg) {
	    assert(msg_in_recv_);
	    *msg = msg_in_recv_;
	    msg_in_recv_ = 0;
	    return 0;
	};
	
    private:
	UDPMessage *recv_msg_;
	std::list<UDPMessage*> send_msg_list_;
	static const int kMaxUDPSendMessageNum = 1000;
    };

};

#endif
