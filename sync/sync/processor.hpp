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

namespace NF {

// 一个Processor对应一个线程
class Processor {
public:
    Processor(Server *srv) {
        srv_ = srv;
    };
    ~Processor() {
    };

    // 线程函数
    void Run(void *arg) {
    };

    // 多路同时发送和接收
    int SendAndRecvMultiSockets(std::vector<int> &sk_list, std::vector<char*> &buf_list,
                                std::vector<int> &buf_len_list) {

    };
private:
    Server *srv_;
    Socket *server_sk_; // 请求来自该套接口
    int epoll_fd_;      // 用于支持同时请求后端服务器
};
}; // namespace NF

#endif // __PROCESSOR_HPP__

