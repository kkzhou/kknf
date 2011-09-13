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

#ifndef __PROCESSOR_HPP__
#define __PROCESSOR_HPP__

#include "server.hpp"
#include "client.hpp"

namespace NF {

// 一个Processor对应一个线程
class Processor {
public:
    Processor(Server *srv, Client *client, uint32_t pool_index)
        :srv_(srv),
		client_(client),
        pool_index_(pool_index){

        ENTERING;
		cancel_ = false;
        max_udp_pkt_size_ = 1472; // 1500 - 20 - 8
        max_tcp_pkt_size_ = 1024 * 1024 * 10; // 10M
        LEAVING;
    };

    virtual ~Processor() {
        ENTERING;
        LEAVING;
    };

    void Stop() {
        ENTERING;
        cancel_ = true;
        LEAVING;
    };
    // 线程函数
    static void* ProcessorThreadProc(void *arg) {

        ENTERING;
        Processor *p = reinterpret_cast<Processor*>(arg);
        p->Process();
        LEAVING;
        return 0;
    };

    // 处理逻辑
    virtual int Process() = 0;

public:
    int max_udp_pkt_size() { return max_udp_pkt_size_;};
    int max_tcp_pkt_size() { return max_tcp_pkt_size_;};
    uint32_t pool_index() { return pool_index_; };
    Server* srv() { return srv_; };
	Client* client() {return client_;}

private:
    Server *srv_;
	Client *client_;
    uint32_t pool_index_;    // 多线程池时的索引

    int max_udp_pkt_size_;
    int max_tcp_pkt_size_;

private:
    Processor(){};
    Processor(Processor&){};
    Processor& operator=(Processor&){ return *this; };

private:
	bool cancel_;
};
}; // namespace NF

#endif // __PROCESSOR_HPP__

