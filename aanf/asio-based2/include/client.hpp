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
#ifndef _CLIENT_HPP_
#define _CLIENT_HPP_

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

#include "socketinfo.hpp"


namespace AANF {

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
    // Async interfaces
    // Called when sending request to the server behind and the connection hasn't established yet.
    int ToConnectThenWrite(TCPEndpoint &remote_endpoint, std::vector<char> &buf_to_send/*content swap*/) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To connect " << remote_endpoint.address().to_string() 
            << ":" << remote_endpoint.port() << std::endl;
        SocketInfoPtr skinfo(new SocketInfo(SocketInfo::T_TCP_LV, io_serv_));
        skinfo->set_remote_endpoint(remote_endpoint);
        skinfo->SetSendBuf(buf_to_send);
        skinfo->set_is_client(true);

        std::cerr << "To add this socket to map" << std::endl;
        InsertTCPClientSocket(remote_endpoint, skinfo);

        skinfo->tcp_sk().async_connect(skinfo->remote_endpoint(),
                boost::bind(
                    &Client::HandleConnectThenWrite,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    skinfo->send_buf()));

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

        std::cerr << "To write " << skinfo->GetFlow() << std::endl;

        boost::asio::async_write(
            skinfo->tcp_sk(),
            boost::asio::buffer(skinfo->send_buf()),
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

        std::cerr << "To destroy " << skinfo->GetFlow() << std::endl;

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
        timer_.async_wait(boost::bind(&Client::HandleTimeout,
                                shared_from_this(),
                                boost::asio::placeholders::error));

        return;
    };

    // Connect handlers
    void HandleConnectThenWrite(const boost::system::error_code& error, SocketInfoPtr skinfo,
                                    std::vector<char> &buf_to_send) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        BOOST_ASSERT(!skinfo->in_use());
        BOOST_ASSERT(skinfo->is_client());
        skinfo->set_in_use(true);

        std::cerr << "To write " << skinfo->GetFlow() << std::endl;

        if (error) {
            std::cerr << "Connect error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        skinfo->set_is_connected(true);
        skinfo->set_in_use(true);

        std::cerr << "To write " << buf_to_send.size() << " bytes" << std::endl;
        skinfo->SetSendBuf(buf_to_send);

        boost::asio::async_write(
            skinfo->tcp_sk(),
            boost::asio::buffer(skinfo->send_buf()),
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
        std::cerr << "To read " << skinfo->GetFlow() << std::endl;
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
                    &Client::HandleReadVThenProcess,
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
    // 2:  this is a client socket, and to close 
    void HandleReadVThenProcess(const boost::system::error_code& error,
                               SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To process " << skinfo->recv_buf().size() << " bytes from " 
            << skinfo->GetFlow() << std::endl;

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
                             SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To read " << skinfo->GetFlow() << std::endl;

        std::cerr << "To read Length_field 4 bytes" << std::endl;

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

#endif
