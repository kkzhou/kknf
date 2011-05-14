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

#ifndef _BASIC_CONNECTION_HPP_
#define _BASIC_CONNECTION_HPP_

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/assert.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include <vector>
#include <list>

#include "message_info.hpp"

namespace AANF {

class BasicConnection : public boost::enable_shared_from_this, private boost::noncopyable {
public:
    typedef std::list<std::pair<boost::shared_ptr<MessageInfo>, boost::functor<BasicConnection*()> > > ProcessResult;
    typedef boost::asio::ip::tcp::socket TCP_Socket;
    typedef boost::asio::ip::address IP_Address;
public:
    BasicConnection(boost::asio::io_service &io_serv, uint32_t init_recv_buffer_size, uint32_t max_recv_buffer_size)
        :socket_(io_serv),
        strand_(io_serv),
        recv_buffer_(new std::vector<char>(init_recv_buffer_size)),
        max_recv_buffer_size_(max_recv_buffer_size),
        in_use_(false),
        conected_(false) {

    };

    TCP_Socket& socket() { return socket_; };

    bool in_use() { return in_use_; };
    bool connected() {return connected_;)};
    uint32_t max_recv_buffer_size() {return max_recv_buffer_size_;};
    boost::shared_ptr<std::vector<char> > recv_buffer() {return recv_buffer_;};
    boost::asio::io_service::strand& strand() {return strand_;};
    void Use() {BOOST_ASSERT(in_use_ == false); in_use_ = true;};
    void UnUse() {BOOST_ASSERT(in_use_ == true); in_use_ = false;};
    void set_connected(bool connected) {connected_ = connected;}

    virtual void StartRead(const boost::system::error_code &ec) = 0;
    virtual void StartWrite(const boost::system::error_code &ec, boost::shared_ptr<MessageInfo> msg) = 0;
    virtual void StartConnect(const boost::system::error_code &ec, std::string &to_ip, uint16_t to_port, boost::shared_ptr<MessageInfo> msg) = 0;
    // handlers for connect
    virtual void HandleConnectThenWrite(const boost::system::error_code &ec, boost::shared_ptr<MessageInfo> msg);
    virtual void HandleConnectThenRead(const boost::system::error_code &ec);
    // handlers for write
    virtual void HandleWriteThenRead(const boost::system::error_code &ec, std::size_t byte_num);
    virtual void HandleWriteThenIdle(const boost::system::error_code &ec, std::size_t byte_num);
    virtual void HandleWriteThenClose(const boost::system::error_code &ec, std::size_t byte_num);
    // handlers for read
    virtual void HandleReadThenRead(const boost::system::error_code &ec, std::size_t byte_num);
    virtual void HandleReadThenProcess(const boost::system::error_code &ec, std::size_t byte_num);


private:
    // 业务逻辑相关的报文处理函数
    // 根据具体业务来实现
    // 返回值是一个列表，列表的元素是要发送的MessageInfo和对应的ConnectionFacotry对。
    // 含义是：在处理这个报文时，需要发出一些报文，为了发出这些报文，需要对应的connection，
    // 而这些connection就是用ConnectionFacotry建立。
    virtual ProcessResult& ProcessMessage(boost::shared_ptr<MessageInfo> msg) = 0;
private:
    TCP_Socket socket_;
    volatile bool in_use_;
    volatile bool connected_;
    boost::shared_ptr<std::vector<char> > recv_buffer_;
    uint32_t max_recv_buffer_size_;

    boost::asio::io_service::strand strand_;
};


};
#endif
