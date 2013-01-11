#ifndef __ADDRES_HPP__
#define __ADDRES_HPP__

nanmespace ZXBNF {

    class Address {
    public:
	static Address* MakeAddress(char *ipstr, unsigned short port) {

	    struct sockaddr_in addr;
	    socklen_t addr_len;
	    bool valid = false;

	    memset(&addr, 0, sizeof(addr_));
	    addr.sin_len = sizeof(addr_) - 8;
	    addr.sin_family = PF_INET;
	    addr.sin_port = htons(port);

	    if (inet_aton(ipstr, &addr.sin_addr) != 1) {
		return 0;
	    }
	    addr_len = sizeof(struct sockaddr);
	    Address *new_address = new Address(addr, addr_len);
	    return new_address;

	};
	Address(struct sockaddr_in addr, socklen_t addr_len) {
	    addr_ = addr;
	    addr_len_ = addr_len;
	};
	Address& operator=(Address &addr) {
	    addr_ = addr;
	    addr_len_ = addr_len;
	    return *this;
	};

	inline struct sockaddr_in &addr() const { return addr_; };
	inline socklen_t &addr_len() const { return addr_len_; };

    private:
	struct sockaddr_in addr_;
	socklen_t addr_len_;
    };
};

#endif
