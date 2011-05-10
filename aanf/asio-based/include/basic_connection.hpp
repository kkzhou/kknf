#ifndef _BASIC_CONNECTION_HPP_
#define _BASIC_CONNECTION_HPP_

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/assert.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread.hpp>

#include <vector>

namespace AANF {

class BasicConnection : public boost::enable_shared_from_this, private boost::noncopyable {
public:
    typedef boost::asio::ip::tcp::socket TCP_Socket;
    typedef boost::asio::ip::address IP_Address;
public:
    BasicConnection(boost::asio::io_service &io_serv, uint32_t init_recv_buffer_size, uint32_t max_recv_buffer_size)
        :socket_(io_serv),
        strand_(io_serv),
        recv_buffer_(new std::vector<char>(init_recv_buffer_size)),
        max_recv_buffer_size_(max_recv_buffer_size),
        in_use_(false) {

    };

    TCP_Socket& socket() { return socket_; };

    bool in_use() { return in_use_; };
    uint32_t max_recv_buffer_size() {return max_recv_buffer_size_;};
    boost::shared_ptr<std::vector<char> > recv_buffer() {return recv_buffer_;};
    boost::asio::io_service::strand& strand() {return strand_;};
    void Use() {BOOST_ASSERT(in_use_ == false); in_use_ = true;};

    void StartRead() = 0;
    void StartWrite(boost::shared_ptr<MessageInfo> msg) = 0;

private:
    TCP_Socket socket_;
    volatile bool in_use_;
    boost::shared_ptr<std::vector<char> > recv_buffer_;
    uint32_t max_recv_buffer_size_;

    boost::asio::io_service::strand strand_;
};


};
#endif
