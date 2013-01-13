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

	inline struct sockaddr_in addr() const { return addr_; };
	inline socklen_t addr_len() const { return addr_len_; };

    private:
	struct sockaddr_in addr_;
	socklen_t addr_len_;
    };
};

#endif
