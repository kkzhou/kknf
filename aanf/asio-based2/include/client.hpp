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

// The key for a socket used in std::map
class SocketKey {
public:
    class SocketKeyCompare {
    public:
        bool operator()(const SocketKey &l, const SocketKey &r) const {
            if (l.ip_ < r.ip_) {
                return true;
            }
            if (l.port_ < r.port_) {
                return true;
            }
            return false;
        };
    };
public:
    SocketKey(std::string &ip, uint16_t port)
        : ip_(ip),
        port_(port) {
    };

    std::string& ip() {
        return ip_;
    };
    uint16_t port() {
        return port_;
    };
private:
    std::string ip_;
    uint16_t port_;
};

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
        in_read_(false),
        in_write_(false),
        is_client_(false),
        is_connected_(false) {

        access_time_ = create_time_ =
            boost::posix_time::microsec_clock::local_time();

    };

    SocketType type() {
        return type_;
    };

    TCPEndpoint& remote_endpoint() {
        return remote_endpoint_;
    };

    void set_remote_endpoint(TCPEndpoint &remote_endpoint) {
        remote_endpoint_ = remote_endpoint;
    };

    bool is_client() {
        return is_client_;
    };

    void set_is_client(bool is_client) {
        is_client_ = is_client;
    };

    bool in_read() {
        return in_read_;
    };

    bool in_write() {
        return in_write_;
    };

    bool is_connected() {
        return is_connected_;
    };

    void set_is_connected(bool is_connected) {
        is_connected_ = is_connected;
    };

    void set_in_read(bool in_read) {
        in_read_ = in_read;
    };

    void set_in_write(bool in_write) {
        in_write_ = in_write;
    };

    TCPSocket& tcp_sk() {
        return tcp_sk_;
    };

    PTime& create_time() {
        return create_time_;
    };

    PTime& access_time() {
        return access_time_;
    };

    void Touch() {
        access_time_ = boost::posix_time::microsec_clock::local_time();
    };

    std::vector<char>& recv_buf() {
        return recv_buf_;
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
    bool in_read_;
    bool in_write_;
    bool is_client_;
    bool is_connected_;
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
        : strand_(io_serv_),
        timer_trigger_interval_(10000),
        timer_(io_serv_),
        init_recv_buf_size_(1024),
        max_recv_buf_size_(1024 * 1024 * 2) {

    };

    // Register handlers to the timer(only one timer)
    void AddTimerHandler(boost::function<void()> func) {

        timer_handler_list_.push_back(func);
        return;

    };

    // Start the server in serveral threads.
    void Run() {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

    
        timer_.expires_from_now(boost::posix_time::milliseconds(timer_trigger_interval_));
        timer_.async_wait(strand().wrap( 
                                boost::bind(&Client::HandleTimeout, 
                                shared_from_this(), 
                                boost::asio::placeholders::error)));

        if (thread_pool_size_ == 1) {
            io_serv_.run();
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        std::list<boost::shared_ptr<boost::thread> > threads;

        for (int i = 0; i < thread_pool_size_; i++) {
            boost::shared_ptr<boost::thread> thread(
                new boost::thread(boost::bind(&boost::asio::io_service::run, &io_serv_)));
            threads.push_back(thread);
        }
        
        std::list<boost::shared_ptr<boost::thread> >::iterator it, endit;
        it = threads.begin();
        endit = threads.end();
        for (; it != endit; it++) {
            (*it)->join();
        }

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
    int ToWriteReadSync(TCPEndpoint &remote_endpoint,
                        std::vector<char> &buf_to_send/*content swap*/,
                        std::vector<char> &buf_to_fill) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
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
        skinfo->set_in_write(true);
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

        skinfo->set_in_write(false);

        // read len_field
        skinfo->set_in_read(true);
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

        skinfo->set_in_read(false);
        std::string ip = skinfo->remote_endpoint().address().to_string();
        SocketKey key(ip, skinfo->remote_endpoint().port());
        ret = InsertTCPClientSocket(key, skinfo);
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Called when sending request to the server behind and the connection has already established.
    int ToWriteReadSync(SocketInfoPtr skinfo, std::vector<char> &buf_to_send,
                        std::vector<char> &buf_to_fill) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        BOOST_ASSERT(skinfo->is_connected());
        BOOST_ASSERT(!skinfo->in_write());
        BOOST_ASSERT(!skinfo->in_read());
        // write
        boost::system::error_code e;
        skinfo->set_in_write(true);
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

        skinfo->set_in_write(false);

        // read len_field
        skinfo->set_in_read(true);
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
            // shared_ptr 'skinfo' will destruct the SocketInfo
            return -3;
        }

        skinfo->set_in_read(false);
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Async interfaces
    // Called when sending request to the server behind and the connection hasn't established yet.
    int ToConnectThenWrite(TCPEndpoint &remote_endpoint, std::vector<char> &buf_to_send/*content swap*/) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        SocketInfoPtr skinfo(new SocketInfo(SocketInfo::T_TCP_LV, io_serv_));
        skinfo->set_remote_endpoint(remote_endpoint);
        skinfo->SetSendBuf(buf_to_send);
        skinfo->set_is_client(true);

        std::string ip = skinfo->remote_endpoint().address().to_string();
        SocketKey key(ip, skinfo->remote_endpoint().port());
        InsertTCPClientSocket(key, skinfo);

        skinfo->tcp_sk().async_connect(skinfo->remote_endpoint(),
            strand().wrap(
                boost::bind(
                    &Client::HandleConnectThenWrite,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    buf_to_send)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };
    // Called when sending request to the server behind and the connection has already established.
    int ToWriteThenRead(SocketInfoPtr skinfo, std::vector<char> &buf_to_send) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        BOOST_ASSERT(skinfo->is_connected());
        BOOST_ASSERT(!skinfo->in_write());
        BOOST_ASSERT(!skinfo->in_read());

        skinfo->set_in_write(true);

        boost::asio::async_write(
            skinfo->tcp_sk(),
            boost::asio::buffer(buf_to_send),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &Client::HandleWriteThenReadL,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

private:
    // Called when beginning to read data from a socket
    int ToReadLThenReadV(SocketInfoPtr skinfo) {

        std::cerr << "Eneter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        BOOST_ASSERT(skinfo->is_connected());
        BOOST_ASSERT(!skinfo->in_write());
        BOOST_ASSERT(!skinfo->in_read());

        skinfo->SetRecvBuf(4);
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf()[0]), 4),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &Client::HandleReadLThenReadV,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    int DestroySocket(SocketInfoPtr skinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        BOOST_ASSERT((!skinfo->is_connected()) || skinfo->in_read() || skinfo->in_write());

        boost::system::error_code e;
        skinfo->tcp_sk().close(e);
        std::string ip = skinfo->remote_endpoint().address().to_string();
        SocketKey key(ip, skinfo->remote_endpoint().port());
        if (skinfo->is_client()) {
            std::map<SocketKey, std::list<SocketInfoPtr> >::iterator it = tcp_client_socket_map_.find(key);

            if (it == tcp_client_socket_map_.end()) {
                std::cerr << "The SocketInfo to delete doesn't exist." << std::endl;
                std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
                return -1;
            }

            it->second.remove(skinfo);
            if (it->second.size() == 0) {
                // the item deleted is the last one
                tcp_client_socket_map_.erase(key);
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
        timer_.async_wait(strand().wrap( 
                                boost::bind(&Client::HandleTimeout, 
                                shared_from_this(), 
                                boost::asio::placeholders::error)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    // Connect handlers
    void HandleConnectThenWrite(const boost::system::error_code& error, SocketInfoPtr skinfo,
                                    std::vector<char> &buf_to_send) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        BOOST_ASSERT(!skinfo->is_connected());
        BOOST_ASSERT(!skinfo->in_write());
        BOOST_ASSERT(!skinfo->in_read());

        if (error) {
            std::cerr << "Connect error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        skinfo->set_is_connected(true);
        std::string ip = skinfo->tcp_sk().remote_endpoint().address().to_string();

        skinfo->set_in_write(true);

        boost::asio::async_write(
            skinfo->tcp_sk(),
            boost::asio::buffer(buf_to_send),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &Client::HandleWriteThenReadL,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    // Read handlers
    void HandleReadLThenReadV(const boost::system::error_code& error,
                            SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        BOOST_ASSERT(skinfo->is_connected());
        BOOST_ASSERT(!skinfo->in_write());
        BOOST_ASSERT(skinfo->in_read());

        if (error) {
            std::cerr << "Read error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            return;
        }

        int len = *(int*)(&((skinfo->recv_buf()[0])));
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
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
            strand().wrap(
                boost::bind(
                    &Client::HandleReadVThenProcess,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;


    };

    void HandleReadVThenProcess(const boost::system::error_code& error,
                               SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        BOOST_ASSERT(skinfo->is_connected());
        BOOST_ASSERT(!skinfo->in_write());
        BOOST_ASSERT(skinfo->in_read());

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

        skinfo->set_in_read(false);
        if (ret < 0) {
            // error
            std::cerr << "ProcessData error" << std::endl;
            DestroySocket(skinfo);

        } else if (ret == 1) {
            // send and close
            DestroySocket(skinfo);
        } else if (ret == 2) {
            // send and then read
            ToReadLThenReadV(skinfo);
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
        BOOST_ASSERT(skinfo->in_write());
        BOOST_ASSERT(!skinfo->in_read());
        BOOST_ASSERT(skinfo->is_connected());

        skinfo->SetRecvBuf(4);  // len_field
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf()[0]), 4),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &Client::HandleReadLThenReadV,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));

        skinfo->set_in_write(false);
        skinfo->set_in_read(true);

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };


protected:

    SocketInfoPtr FindIdleTCPClientSocket(SocketKey &key) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        SocketInfoPtr ret;
        std::map<SocketKey, std::list<SocketInfoPtr> >::iterator it
            = tcp_client_socket_map_.find(key);

        if (it != tcp_client_socket_map_.end()) {
            std::list<SocketInfoPtr>::iterator list_it =
                it->second.begin();
            while (list_it != it->second.end()) {
                if ((*list_it)->in_write() || (*list_it)->in_read()) {
                    list_it++;
                } else {
                    break;
                }
            } // while
            ret = (*list_it);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return ret;
    };

    int InsertTCPClientSocket(SocketKey &key, SocketInfoPtr new_skinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        BOOST_ASSERT(!new_skinfo->in_write());
        BOOST_ASSERT(!new_skinfo->in_read());

        std::map<SocketKey, std::list<SocketInfoPtr> >::iterator it
            = tcp_client_socket_map_.find(key);

        if (it != tcp_client_socket_map_.end()) {
            it->second.push_back(new_skinfo);
        } else {
            std::list<SocketInfoPtr> new_list;
            new_list.push_back(new_skinfo);
            std::pair<std::map<SocketKey, std::list<SocketInfoPtr> >::iterator, bool> insert_result =
                tcp_client_socket_map_.insert(std::pair<SocketKey, std::list<SocketInfoPtr> >(key, new_list));

            BOOST_ASSERT(insert_result.second);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };
public:
    // setter/getter
    boost::asio::io_service::strand& strand() {
        return strand_;
    };
    int thread_pool_size() {
        return thread_pool_size_;
    };
    void set_thread_pool_size(int thread_pool_size) {
        thread_pool_size_ = thread_pool_size;
    };
    int max_recv_buf_size() {
        return max_recv_buf_size_;
    };
    void set_max_recv_buf_size(int max_recv_buf_size) {
        max_recv_buf_size_ = max_recv_buf_size;
    };
    int init_recv_buf_size(){
        return init_recv_buf_size_;
    };
    void set_init_recv_buf_size(int init_recv_buf_size) {
        init_recv_buf_size_ = init_recv_buf_size;
    };

    void set_timer_trigger_interval(int millisec) {
        timer_trigger_interval_ = millisec;
    };
    int timer_trigger_interval() {
        return timer_trigger_interval_;
    };
private:
    // io_service
    boost::asio::io_service io_serv_;
    // Data structure to maintain sockets(connections)
    std::map<SocketKey, std::list<SocketInfoPtr>, SocketKey::SocketKeyCompare > tcp_client_socket_map_;
    // strand is for synchronous
    boost::asio::strand strand_;
    // timer
    int timer_trigger_interval_;
    boost::asio::deadline_timer timer_;
    // thread
    int thread_pool_size_;

    // timer handlers
    std::list<boost::function<void()> > timer_handler_list_;
    // socket parameters
    int init_recv_buf_size_;
    int max_recv_buf_size_;
};

};// namespace
