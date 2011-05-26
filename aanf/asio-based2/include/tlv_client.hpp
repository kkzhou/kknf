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
typedef boost::asio::ip::udp::socket UDPSocket;
typedef boost::shared_ptr<UDPSocket> UDPSocketPtr;
typedef boost::asio::ip::udp::endpoint UDPEndpoint;
typedef boost::posix_time::ptime PTime;

class TCPSocketInfo;
typedef boost::shared_ptr<TCPSocketInfo> TCPSocketInfoPtr;

class UDPSocketInfo;
typedef boost::shared_ptr<UDPSocketInfo> UDPSocketInfoPtr;

// Simple information about a socket
class UDPSocketInfo {
public:
    UDPSocketInfo(boost::asio::io_service &io_serv)
        : max_length_of_send_buf_list_(100),
        udp_sk_(io_serv),
        in_use_(false) {

        access_time_ = create_time_ =
            boost::posix_time::microsec_clock::local_time();
    };

    UDPEndpoint& local_endpoint() { return local_endpoint_; };
    void set_in_use(bool in_use) { in_use_ = in_use;};
    bool in_use() {return in_use_;};
    UDPSocket& udp_sk() { return udp_sk_; };
    PTime& create_time() { return create_time_; };
    PTime& access_time() { return access_time_; };
    std::vector<char>& recv_buf() { return recv_buf_; };
    std::vector<char>& send_buf() { return send_buf_; };
    void set_max_length_of_send_buf_list(int max) {max_length_of_send_buf_list_ = max;};

    void Touch() {
        access_time_ = boost::posix_time::microsec_clock::local_time();
    };

    void SetRecvBuf(size_t byte_num) {
        std::vector<char> tmp(byte_num);
        recv_buf_.swap(tmp);
    };

    void SetRecvBuf(std::vector<char> &new_buf) {
        recv_buf_.swap(new_buf);
    };

    int AddSendBuf(UDPEndpoint to_endpoint, std::vector<char> &to_send) {

        std::pair<UDPEndpoint, std::vector<char> > new_item;
        if (send_buf_list_.size() == max_length_of_send_buf_list_) {
            return -1;
        }

        new_item.first = to_endpoint;
        send_buf_list_.push_back(new_item);
        send_buf_list_.back().second.swap(to_send);
        return 0;
    };

    std::pair<UDPEndpoint, std::vector<char> >&
        GetSendBufListFront() {

        return send_buf_list_.front();
    };

    void DeleteSendBufListFront() {

        return send_buf_list_.pop_front();
    };

    size_t GetSendBufListLength() {
        return send_buf_list_.size();
    };

private:
    int max_length_of_send_buf_list_;
    UDPEndpoint local_endpoint_;
    UDPSocket udp_sk_;
    PTime create_time_;
    PTime access_time_;
    bool in_use_;
    std::vector<char> recv_buf_;
    std::list<std::pair<UDPEndpoint, std::vector<char> > > send_buf_list_;
};

class TCPSocketInfo {
public:
    TCPSocketInfo(boost::asio::io_service &io_serv)
        : type_(type),
        tcp_sk_(io_serv),
        is_client_(false),
        is_connected_(false),
        in_use_(false) {

        access_time_ = create_time_ =
            boost::posix_time::microsec_clock::local_time();
    };

    TCPEndpoint& remote_endpoint() { return remote_endpoint_; };
    void set_remote_endpoint(TCPEndpoint &remote_endpoint) { remote_endpoint_ = remote_endpoint;};
    TCPEndpoint& local_endpoint() { return local_endpoint_; };
    void set_local_endpoint(TCPEndpoint &local_endpoint) { local_endpoint_ = local_endpoint;};
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
    std::vector<char>& send_buf() { return send_buf_; };

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
    TCPEndpoint local_endpoint_;
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
class HTTPClient : public boost::enable_shared_from_this<HTTPClient> {

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
    HTTPClient()
        : timer_trigger_interval_(10000),
        timer_(io_serv_),
        init_recv_buf_size_(1024),
        max_recv_buf_size_(1024 * 1024 * 2) {

    };
    virtual ~HTTPClient(){};

    // Register handlers to the timer(only one timer)
    void AddTimerHandler(boost::function<void()> func) {

        timer_handler_list_.push_back(func);
        std::cerr << "A new timer handler is added" << std::endl;
        return;

    };

    int InitUDPSocket(UDPEndpoint local_endpoint) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        udp_socket_.reset(new UDPSocketInfo(io_serv_));
        boost::system::error_code e;
        udp_socket_->upd_sk().bind(local_endpoint, e);

        if (e) {
            std::cerr << "Bind UDP socket error: " << e.message() << std::endl;
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Start the server in serveral threads.
    void Run() {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "Start a timer in " << timer_trigger_interval_ << " milliseconds" << std::endl;
        timer_.expires_from_now(boost::posix_time::milliseconds(timer_trigger_interval_));
        timer_.async_wait(boost::bind(&HTTPClient::HandleTimeout,
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
    // Async interfaces
    // Called when sending request to the server behind and the connection hasn't established yet.
    int ToConnectThenWrite(TCPEndpoint &remote_endpoint, std::vector<char> &buf_to_send/*content swap*/) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To connect " << remote_endpoint.address().to_string()
            << ":" << remote_endpoint.port() << std::endl;
        TCPSocketInfoPtr skinfo(new TCPSocketInfo(io_serv_));
        skinfo->set_remote_endpoint(remote_endpoint);
        skinfo->SetSendBuf(buf_to_send);
        skinfo->set_is_client(true);

        std::cerr << "To add this socket to map" << std::endl;
        InsertTCPClientSocket(remote_endpoint, skinfo);

        skinfo->tcp_sk().async_connect(skinfo->remote_endpoint(),
                boost::bind(
                    &HTTPClient::HandleConnectThenWrite,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    skinfo->send_buf()));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Called when sending request to the server behind and the connection has already established.
    int ToWriteThenRead(TCPSocketInfoPtr skinfo, std::vector<char> &buf_to_send) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        if (skinfo->is_client()) {
            BOOST_ASSERT(!skinfo->in_use());
            skinfo->set_in_use(true);
        }

        std::cerr << "To write " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;

        boost::asio::async_write(
            skinfo->tcp_sk(),
            boost::asio::buffer(skinfo->send_buf()),
            boost::asio::transfer_all(),
                boost::bind(
                    &HTTPClient::HandleWriteThenReadL,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

private:
    int UDPToWrite(UDPEndpoint to_endpoint, std::vector<char> &buf_to_send) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To write " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;

        int ret = udp_socket_->AddSendBuf(to_endpoint, buf_to_send);
        if (ret < 0) {
            std::cerr << "Send buffer list is full " << std::endl;
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }

        if (!udp_socket_->in_use()) {
            // not in sending, so async_send_to

            std::pair<UDPEndpoint, std::vector<char> > > ret =
                udp_socket_->GetSendBufListFront();
            std::cerr << "To write " << ret.second.size() << " bytes" << std::endl;
            udp_socket_->async_send_to(boost::asio::buffer(ret.second),
                                       ret.first,
                                       boost::bind(&HTTPClient::HandleUDPWrite,
                                                   shared_from_this(),
                                                   udp_socket_,
                                                   boost::asio::placeholders::bytes_transferred));
            udp_socket_->set_in_use(true);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    int DestroySocket(TCPSocketInfoPtr skinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To destroy " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;

        boost::system::error_code e;
        skinfo->tcp_sk().close(e);

        if (skinfo->is_client()) {
            std::map<TCPEndpoint, std::list<TCPSocketInfoPtr> >::iterator it
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

        std::cerr << "Fire timer" << std::endl;
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

        std::cerr << "Reload timer" << std::endl;
        timer_.expires_from_now(boost::posix_time::milliseconds(timer_trigger_interval_));
        timer_.async_wait(boost::bind(&HTTPClient::HandleTimeout,
                                shared_from_this(),
                                boost::asio::placeholders::error));

        return;
    };

    // Connect handlers
    void HandleConnectThenWrite(const boost::system::error_code& error, TCPSocketInfoPtr skinfo,
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
        boost::system::error_code e;
        TCPEndpoint local_endpoint = skinfo->local_endpoint(e);
        if (e) {
            std::cerr << "Get localendpoint error: " << e.message() << std::endl;
            DestroySocket(skinfo);
            return;
        }
        skinfo->set_local_endpoint(local_endpoint);

        std::cerr << "To write " << buf_to_send.size() << " bytes" << std::endl;
        skinfo->SetSendBuf(buf_to_send);

        boost::asio::async_write(
            skinfo->tcp_sk(),
            boost::asio::buffer(skinfo->send_buf()),
            boost::asio::transfer_all(),
                boost::bind(
                    &HTTPClient::HandleWriteThenReadL,
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

        std::cerr << "To read" << len << " bytes" << std::endl;
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf().at(4)), len - 4),
            boost::asio::transfer_all(),
                boost::bind(
                    &HTTPClient::HandleReadVThenProcess,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    // The whold packet has been read into skinfo->recv_buf_, then call
    // the virtual method ProcessData to process the packet.
    // return value:
    // <0:  error happened
    // 0:  this is a server socket, and to close it
    // 1:  this is a client socket, and set it idle
    // 2:  this is a server socket, and to read
    void HandleReadVThenProcess(const boost::system::error_code& error,
                               TCPSocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To process " << skinfo->recv_buf().size() << " bytes from "
            << skinfo->remote_endpoint().address().to_string() << ":"
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

        std::string fromip = skinfo->remote_endpoint().address().to_string();
        std::string toip = skinfo->local_endpoint().address().to_string();
        int ret = ProcessData(skinfo->recv_buf(),
                                fromip,
                                skinfo->remote_endpoint().port(),
                                toip,
                                skinfo->local_endpoint().port(),
                                boost::posix_time::microsec_clock::local_time());

        if (ret < 0) {
            // error
            std::cerr << "ProcessData error" << std::endl;
            DestroySocket(skinfo);
        } else if (ret == 0) {
            // this is a server socket, to close after send
            BOOST_ASSERT(false); // not happen in client.hpp
            BOOST_ASSERT(!skinfo->is_client());
        } else if (ret == 1) {
            // this is a client socket, to send and idle
            BOOST_ASSERT(skinfo->in_use());
            BOOST_ASSERT(skinfo->is_client());
            skinfo->set_in_use(false);
        } else if (ret == 2) {
            // this is a client socket, to send and close
            BOOST_ASSERT(skinfo->in_use());
            BOOST_ASSERT(skinfo->is_client());
        } else {
            BOOST_ASSERT(false);
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

    // Write handlers
    void HandleWriteThenReadL(const boost::system::error_code& error,
                             TCPSocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To read " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;

        std::cerr << "To read Length_field 4 bytes" << std::endl;

        skinfo->SetRecvBuf(4);  // len_field

        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf()[0]), 4),
            boost::asio::transfer_all(),
                boost::bind(
                    &HTTPClient::HandleReadLThenReadV,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    // UDP write handler
    void HandleUDPWrite(const boost::system::error_code& error,
                             UDPSocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To write " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;

        if (skinfo->GetSendBufListLength() != 0) {

            std::pair<UDPEndpoint, std::vector<char> > > ret =
                udp_socket_->GetSendBufListFront();
            std::cerr << "To write " << ret.second.size() << " bytes" << std::endl;
            udp_socket_->async_send_to(boost::asio::buffer(ret.second),
                                       ret.first,
                                       boost::bind(&HTTPClient::HandleUDPWrite,
                                                   shared_from_this(),
                                                   udp_socket_,
                                                   boost::asio::placeholders::bytes_transferred));
        } else {
            BOOST_ASSERT(udp_socket_->in_use());
            udp_socket_->set_in_use(false);
        }


        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

protected:

    TCPSocketInfoPtr FindIdleTCPClientSocket(TCPEndpoint &key) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To find " << key.address().to_string() << ":" << key.port() << std::endl;
        TCPSocketInfoPtr ret;
        std::map<TCPEndpoint, std::list<TCPSocketInfoPtr> >::iterator it
            = tcp_client_socket_map_.find(key);

        if (it != tcp_client_socket_map_.end()) {
            std::list<TCPSocketInfoPtr>::iterator list_it =
                it->second.begin();
            std::cerr << "There are " << it->second.size() << " sockets for this <ip, port> in use." << std::endl;
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
            std::cerr << "No idle socket found" << std::endl;
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return ret;
    };

    int InsertTCPClientSocket(TCPEndpoint &key, TCPSocketInfoPtr new_skinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To insert " << key.address().to_string() << ":" << key.port() << std::endl;

        std::map<TCPEndpoint, std::list<TCPSocketInfoPtr> >::iterator it
            = tcp_client_socket_map_.find(key);

        if (it != tcp_client_socket_map_.end()) {
            std::cerr << "There are " << it->second.size() << " sockets for this <ip, port> in use." << std::endl;
            it->second.push_back(new_skinfo);
        } else {
            std::cerr << "There is no socket for this <ip, port>." << std::endl;
            std::list<TCPSocketInfoPtr> new_list;
            new_list.push_back(new_skinfo);
            std::pair<std::map<TCPEndpoint, std::list<TCPSocketInfoPtr> >::iterator, bool> insert_result =
                tcp_client_socket_map_.insert(std::pair<TCPEndpoint, std::list<TCPSocketInfoPtr> >(key, new_list));

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
    UDPSocketInfoPtr udp_socket_;
    std::map<TCPEndpoint, std::list<TCPSocketInfoPtr> > tcp_client_socket_map_;
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
