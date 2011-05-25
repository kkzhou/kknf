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

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <map>
#include <list>
#include <vector>
#include <iostream>


namespace AANF {

typedef boost::asio::ip::address IPAddress;
typedef boost::asio::ip::tcp::socket TCPSocket;
typedef boost::shared_ptr<TCPSocket> TCPSocketPtr;
typedef boost::asio::ip::tcp::endpoint TCPEndpoint;
typedef boost::posix_time::ptime PTime;

class SocketInfo;
typedef boost::shared_ptr<SocketInfo> SocketInfoPtr;

// Simple information about a socket
class SocketInfo {
public:
    enum SocketType {
        T_UDP,          // UDP is datagram no need to packetize
        T_TCP_LINE,     // '\n to terminate a packet
        T_TCP_LV,       // the first 4 bypes is 'length' field
        T_TCP_TLV,      // the first 4 bytes is 'type' and the second 4 bytes is 'length'
        T_TCP_HTTP,     // http packet
        T_TCP_USR1,
        T_TCP_UNKNOWN
    };


public:
    SocketInfo(SocketType type, boost::asio::io_service &io_serv)
        : type_(type),
        tcp_sk_(io_serv),
        is_client_(false),
        is_connected_(false),
        in_use_(false) {

        access_time_ = create_time_ =
            boost::posix_time::microsec_clock::local_time();
    };

    SocketType type() { return type_; };
    TCPEndpoint& remote_endpoint() { return remote_endpoint_; };

    void set_remote_endpoint(TCPEndpoint &remote_endpoint) { remote_endpoint_ = remote_endpoint;};
    bool is_connected() { return is_connected_; };
    void set_is_connected(bool is_connected) { is_connected_ = is_connected; };
    void set_in_use(bool in_use) { in_use_ = in_use;};
    bool in_use() {return in_use_;};
    bool is_client() { return is_client_; };
    void set_is_client(bool is_client) { is_client_ = is_client; };
    TCPSocket& tcp_sk() { return tcp_sk_; };
    PTime& create_time() { return create_time_; };
    PTime& access_time() { return access_time_; };
    std::vector<char>& recv_buf() { return recv_buf_; };

    void Touch() {
        access_time_ = boost::posix_time::microsec_clock::local_time();
    };

    // Obtain the content using 'swap'
    void GetData(std::vector<char> &to_swap) {
        recv_buf_.swap(to_swap);
    };

    void SetRecvBuf(size_t byte_num) {
        std::vector<char> tmp(byte_num);
        recv_buf_.swap(tmp);
    };

    void SetRecvBuf(std::vector<char> &new_buf) {
        recv_buf_.swap(new_buf);
    };

    void SetSendBuf(std::vector<char> &to_send) {
        send_buf_.swap(to_send);
    };

private:
    TCPEndpoint remote_endpoint_;
    SocketType type_;
    TCPSocket tcp_sk_;
    PTime create_time_;
    PTime access_time_;
    bool is_client_;
    bool is_connected_;
    bool in_use_;
    std::vector<char> recv_buf_;
    std::vector<char> send_buf_;

};

// The skeleton of a client
class Client : public boost::enable_shared_from_this<Client> {

public:
    // Two interfaces, the only two should be overrided in derived classes
    // Process data
    virtual int ProcessData(std::vector<char> &input_data, std::string &from_ip, uint16_t from_port,
                    std::string &to_ip, uint16_t to_port, PTime arrive_time) {
        BOOST_ASSERT(false);
        return 0;
    };
    virtual void PrepareDataThenSend() {
        BOOST_ASSERT(false);
        return;
    };

public:
    Client()
        : timer_trigger_interval_(10000),
        timer_(io_serv_),
        init_recv_buf_size_(1024),
        max_recv_buf_size_(1024 * 1024 * 2) {

    };
    virtual ~Client(){};

    // Register handlers to the timer(only one timer)
    void AddTimerHandler(boost::function<void()> func) {

        timer_handler_list_.push_back(func);
        std::cerr << "A new timer handler is added" << std::endl;
        return;

    };

    // Start the server in serveral threads.
    void Run() {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "Start a timer in " << timer_trigger_interval_ << " milliseconds" << std::endl;
        timer_.expires_from_now(boost::posix_time::milliseconds(timer_trigger_interval_));
        timer_.async_wait(boost::bind(&Client::HandleTimeout,
                                shared_from_this(),
                                boost::asio::placeholders::error));
        io_serv_.run();
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    void Stop() {
        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        io_serv_.stop();
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

public:
    // Sync interfaces
    // Called when sending request to the server behind and the connection hasn't established yet.
    int ToWriteThenReadSync(TCPEndpoint &remote_endpoint,
                        std::vector<char> &buf_to_send/*content swap*/,
                        std::vector<char> &buf_to_fill) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To connect " << remote_endpoint.address().to_string() << ":" << remote_endpoint.port() << std::endl;
        SocketInfoPtr skinfo(new SocketInfo(SocketInfo::T_TCP_LV, io_serv_));

        // connect
        skinfo->set_remote_endpoint(remote_endpoint);
        boost::system::error_code e;
        skinfo->tcp_sk().connect(remote_endpoint, e);
        if (e) {
            std::cerr << "Connect error: " << e.message() << std::endl;
            // shared_ptr 'skinfo' will destruct the SocketInfo
            return -1;
        }
        skinfo->set_is_connected(true);

        // write
        size_t ret = boost::asio::write(skinfo->tcp_sk(),
                                        boost::asio::buffer(buf_to_send),
                                        boost::asio::transfer_all(),
                                        e);
        if (e) {
            std::cerr << "Write error: " << e.message() << std::endl;
            // shared_ptr 'skinfo' will destruct the SocketInfo
            return -1;
        }

        BOOST_ASSERT(ret == buf_to_send.size());

        // read len_field
        std::vector<char> len_field(4, 0);
        int len = 0;

        ret = boost::asio::read(skinfo->tcp_sk(),
                                boost::asio::buffer(len_field),
                                boost::asio::transfer_all(),
                                e);

        if (e) {
            std::cerr << "Read len_field error: " << e.message() << std::endl;
            // shared_ptr 'skinfo' will destruct the SocketInfo
            return -1;
        }
        BOOST_ASSERT(ret == 4);

        len = *reinterpret_cast<int*>(&len_field[0]);
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        if (len < 0 || len > max_recv_buf_size()) {
            std::cerr << "len_field invalid: " << len << std::endl;
            return -2;
        }
        buf_to_fill.resize(len);
        std::copy(len_field.begin(), len_field.end(), buf_to_fill.begin());
        ret = boost::asio::read(skinfo->tcp_sk(),
                                boost::asio::buffer(&len_field[4], len - 4),
                                boost::asio::transfer_all(),
                                e);

        if (e) {
            std::cerr << "Read data_field error: " << e.message() << std::endl;
            // shared_ptr 'skinfo' will destruct the SocketInfo
            return -3;
        }

        InsertTCPClientSocket(remote_endpoint, skinfo);
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Called when sending request to the server behind and the connection has already established.
    int ToWriteThenReadSync(SocketInfoPtr skinfo, std::vector<char> &buf_to_send,
                        std::vector<char> &buf_to_fill) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To write " << skinfo->remote_endpoint().address().to_string() 
            << ":" << skinfo->remote_endpoint().port() << std::endl;
        // write
        boost::system::error_code e;
        size_t ret = boost::asio::write(skinfo->tcp_sk(),
                                        boost::asio::buffer(buf_to_send),
                                        boost::asio::transfer_all(),
                                        e);
        if (e) {
            std::cerr << "Write error: " << e.message() << std::endl;
            DestroySocket(skinfo);
            return -1;
        }

        BOOST_ASSERT(ret == buf_to_send.size());

        // read len_field
        std::vector<char> len_field(4, 0);
        int len = 0;

        ret = boost::asio::read(skinfo->tcp_sk(),
                                boost::asio::buffer(len_field),
                                boost::asio::transfer_all(),
                                e);

        if (e) {
            std::cerr << "Read len_field error: " << e.message() << std::endl;
            DestroySocket(skinfo);
            return -1;
        }
        BOOST_ASSERT(ret == 4);

        len = *reinterpret_cast<int*>(&len_field[0]);
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        if (len < 0 || len > max_recv_buf_size()) {
            std::cerr << "len_field invalid: " << len << std::endl;
            DestroySocket(skinfo);
            return -2;
        }

        buf_to_fill.resize(len);
        std::copy(len_field.begin(), len_field.end(), buf_to_fill.begin());
        ret = boost::asio::read(skinfo->tcp_sk(),
                                boost::asio::buffer(&buf_to_fill[4], len - 4),
                                boost::asio::transfer_all(),
                                e);

        if (e) {
            std::cerr << "Read data_field error: " << e.message() << std::endl;
            DestroySocket(skinfo);
            return -3;
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Async interfaces
    // Called when sending request to the server behind and the connection hasn't established yet.
    int ToConnectThenWrite(TCPEndpoint &remote_endpoint, std::vector<char> &buf_to_send/*content swap*/) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To connect " << remote_endpoint.address().to_string() << ":" << remote_endpoint.port() << std::endl;
        SocketInfoPtr skinfo(new SocketInfo(SocketInfo::T_TCP_LV, io_serv_));
        skinfo->set_remote_endpoint(remote_endpoint);
        skinfo->SetSendBuf(buf_to_send);
        skinfo->set_is_client(true);

        InsertTCPClientSocket(remote_endpoint, skinfo);

        skinfo->tcp_sk().async_connect(skinfo->remote_endpoint(),
                boost::bind(
                    &Client::HandleConnectThenWrite,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    buf_to_send));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Called when sending request to the server behind and the connection has already established.
    int ToWriteThenRead(SocketInfoPtr skinfo, std::vector<char> &buf_to_send) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        if (skinfo->is_client()) {
            BOOST_ASSERT(!skinfo->in_use());
            skinfo->set_in_use(true);
        }
        std::cerr << "To write " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;
        boost::asio::async_write(
            skinfo->tcp_sk(),
            boost::asio::buffer(buf_to_send),
            boost::asio::transfer_all(),
                boost::bind(
                    &Client::HandleWriteThenReadL,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

private:

    int DestroySocket(SocketInfoPtr skinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To destroy " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;

        boost::system::error_code e;
        skinfo->tcp_sk().close(e);

        if (skinfo->is_client()) {
            std::map<TCPEndpoint, std::list<SocketInfoPtr> >::iterator it
                = tcp_client_socket_map_.find(skinfo->remote_endpoint());

            if (it == tcp_client_socket_map_.end()) {
                std::cerr << "The SocketInfo to delete doesn't exist." << std::endl;
                std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
                return -1;
            }

            it->second.remove(skinfo);
            if (it->second.size() == 0) {
                // the item deleted is the last one
                tcp_client_socket_map_.erase(skinfo->remote_endpoint());
            }
        } else {
            BOOST_ASSERT(false);
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };
private:
    // timer handler
    void HandleTimeout(const boost::system::error_code& error) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        if (error) {
            std::cerr << "Timer error: " << error.message() << std::endl;
        } else {
            std::list<boost::function<void()> >::iterator it, endit;
            it = timer_handler_list_.begin();
            endit = timer_handler_list_.end();
            for (; it != endit; it++) {
                (*it)();
            }
        }

        timer_.expires_from_now(boost::posix_time::milliseconds(timer_trigger_interval_));
        timer_.async_wait(boost::bind(&Client::HandleTimeout,
                                shared_from_this(),
                                boost::asio::placeholders::error));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    // Connect handlers
    void HandleConnectThenWrite(const boost::system::error_code& error, SocketInfoPtr skinfo,
                                    std::vector<char> &buf_to_send) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        BOOST_ASSERT(!skinfo->in_use());
        BOOST_ASSERT(skinfo->is_client());
        skinfo->set_in_use(true);

        std::cerr << "To write " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;
        if (error) {
            std::cerr << "Connect error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        skinfo->set_is_connected(true);
        skinfo->set_in_use(true);

        boost::asio::async_write(
            skinfo->tcp_sk(),
            boost::asio::buffer(buf_to_send),
            boost::asio::transfer_all(),
                boost::bind(
                    &Client::HandleWriteThenReadL,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    // Read handlers
    void HandleReadLThenReadV(const boost::system::error_code& error,
                            SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To read " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;
        if (error) {
            std::cerr << "Read error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            return;
        }

        int len = *(int*)(&((skinfo->recv_buf()[0])));
        std::cerr << "Length_field is " << len << std::endl;
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        std::cerr << "Length is " << len << std::endl;
        if (len > max_recv_buf_size_) {
            std::cerr << "Data length is too large, so close: " << len << ">" << max_recv_buf_size_ << std::endl;
            DestroySocket(skinfo);
            return;
        }

        if (len > static_cast<int>(skinfo->recv_buf().size())) {
            skinfo->SetRecvBuf(len);
            *(int*)(&skinfo->recv_buf().at(0)) = boost::asio::detail::socket_ops::network_to_host_long(len);
        }

        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf().at(4)), len - 4),
            boost::asio::transfer_all(),
                boost::bind(
                    &Client::HandleReadVThenProcess,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;


    };

    void HandleReadVThenProcess(const boost::system::error_code& error,
                               SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To process " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;

        if (error) {
            std::cerr << "ReadV error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        int len = *(int*)(&((skinfo->recv_buf().at(0))));
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        BOOST_ASSERT(static_cast<int>(byte_num) == len - 4);

        std::string fromip = skinfo->tcp_sk().remote_endpoint().address().to_string();
        std::string toip = skinfo->tcp_sk().local_endpoint().address().to_string();
        int ret = ProcessData(skinfo->recv_buf(),
                                fromip,
                                skinfo->tcp_sk().remote_endpoint().port(),
                                toip,
                                skinfo->tcp_sk().local_endpoint().port(),
                                boost::posix_time::microsec_clock::local_time());

        if (ret < 0) {
            // error
            std::cerr << "ProcessData error" << std::endl;
            DestroySocket(skinfo);
        } else if (ret == 1) {
            // send and idle
            BOOST_ASSERT(skinfo->in_use());
            BOOST_ASSERT(skinfo->is_client());
            skinfo->set_in_use(false);
        } else {
            BOOST_ASSERT(false);
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

    // Write handlers
    void HandleWriteThenReadL(const boost::system::error_code& error,
                             SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To read " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;

        skinfo->SetRecvBuf(4);  // len_field
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf()[0]), 4),
            boost::asio::transfer_all(),
                boost::bind(
                    &Client::HandleReadLThenReadV,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };


protected:

    SocketInfoPtr FindIdleTCPClientSocket(TCPEndpoint &key) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To find " << key.address().to_string() << ":" << key.port() << std::endl;
        SocketInfoPtr ret;
        std::map<TCPEndpoint, std::list<SocketInfoPtr> >::iterator it
            = tcp_client_socket_map_.find(key);

        if (it != tcp_client_socket_map_.end()) {
            std::list<SocketInfoPtr>::iterator list_it =
                it->second.begin();
            while (list_it != it->second.end()) {
                if ((*list_it)->in_use() || (!(*list_it)->is_connected())) {
                    list_it++;
                } else {
                    ret = (*list_it);
                    std::cerr << "Found" << std::endl;
                    break;
                }
            } // while
        }
        if (!ret) {
            std::cerr << "Not found" << std::endl;
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return ret;
    };

    int InsertTCPClientSocket(TCPEndpoint &key, SocketInfoPtr new_skinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To insert " << key.address().to_string() << ":" << key.port() << std::endl;

        std::map<TCPEndpoint, std::list<SocketInfoPtr> >::iterator it
            = tcp_client_socket_map_.find(key);

        if (it != tcp_client_socket_map_.end()) {
            std::cerr << "There are " << it->second.size() << " sockets for this <ip, port> in use." << std::endl;
            it->second.push_back(new_skinfo);
        } else {
            std::cerr << "There is no socket for this <ip, port>." << std::endl;
            std::list<SocketInfoPtr> new_list;
            new_list.push_back(new_skinfo);
            std::pair<std::map<TCPEndpoint, std::list<SocketInfoPtr> >::iterator, bool> insert_result =
                tcp_client_socket_map_.insert(std::pair<TCPEndpoint, std::list<SocketInfoPtr> >(key, new_list));

            BOOST_ASSERT(insert_result.second);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

public:
    // setter/getter
    int max_recv_buf_size() { return max_recv_buf_size_; };
    void set_max_recv_buf_size(int max_recv_buf_size) { max_recv_buf_size_ = max_recv_buf_size; };
    int init_recv_buf_size(){ return init_recv_buf_size_; };
    void set_init_recv_buf_size(int init_recv_buf_size) { init_recv_buf_size_ = init_recv_buf_size; };
    void set_timer_trigger_interval(int millisec) { timer_trigger_interval_ = millisec; };
    int timer_trigger_interval() { return timer_trigger_interval_; };
private:
    // io_service
    boost::asio::io_service io_serv_;
    // Data structure to maintain sockets(connections)
    std::map<TCPEndpoint, std::list<SocketInfoPtr> > tcp_client_socket_map_;
    // timer
    int timer_trigger_interval_;
    boost::asio::deadline_timer timer_;

    // timer handlers
    std::list<boost::function<void()> > timer_handler_list_;
    // socket parameters
    int init_recv_buf_size_;
    int max_recv_buf_size_;
};

};// namespace
