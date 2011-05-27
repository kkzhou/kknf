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

#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <stdlib.h>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <map>
#include <list>
#include <vector>
#include <iostream>

#include "socketinfo.hpp"

namespace AANF {

// The skeleton of a server
class Server : public boost::enable_shared_from_this<Server> {

public:
    // The only interface should be overrided in derived classes to deal with business logic
    // Process data
    virtual int ProcessData(std::vector<char> &input_data, std::string &from_ip, uint16_t from_port,
                    std::string &to_ip, uint16_t to_port, PTime arrive_time) {
        BOOST_ASSERT(false);
        return 0;
    };

public:
    Server()
        : timer_trigger_interval_(10000),
        timer_(io_serv_),
        init_recv_buf_size_(1024),
        max_recv_buf_size_(1024 * 1024 * 2),
        tcp_backlog_(1024),
        server_timeout_(10000) {

    };
    virtual ~Server(){};

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
        udp_socket_->set_local_endpoint(local_endpoint);
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Register handlers to the timer(only one timer)
    // which means add a handler to a callback chain
    void AddTimerHandler(boost::function<void()> func) {

        timer_handler_list_.push_back(func);
        return;

    };

    // Add listen sockets to accept connections
    int AddTCPAcceptor(TCPEndpoint endpoint, SocketInfo::SocketType type) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To listen " << endpoint.address().to_string() << ":"
            << endpoint.port() << std::endl;

        TCPAcceptorInfoPtr new_acceptorinfo(new TCPAcceptorInfo(type, io_serv_));
        new_acceptorinfo->acceptor().open(endpoint.protocol());
        new_acceptorinfo->acceptor().set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        new_acceptorinfo->acceptor().bind(endpoint);
        new_acceptorinfo->acceptor().listen(tcp_backlog_);

        // Firstly, we should add the 'TCPAcceptorInfoPtr' in the global std::map to
        // prevent it from destructing.
        std::cerr << "To add the acceptor to map" << std::endl;
        int ret = InsertTCPAcceptor(endpoint, new_acceptorinfo);
        if (ret < 0) {
            std::cerr << "The TCPAcceptor already exists! (Can't be)" << std::endl;
            exit(1);
        }

        std::cerr << "The acceptor is added, to accept" << std::endl;
        new_acceptorinfo->acceptor().async_accept(new_acceptorinfo->sk_info()->tcp_sk(),
                boost::bind(
                    &Server::HandleAccept,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    new_acceptorinfo));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Start the server in serveral threads.
    void Run() {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        AddTimerHandler(boost::function<void()>(
            boost::bind(&Server::SweepServerSocket,
                shared_from_this(),
                server_timeout_)));

        timer_.expires_from_now(boost::posix_time::milliseconds(timer_trigger_interval_));
        timer_.async_wait( boost::bind(&Server::HandleTimeout,
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
        std::cerr << "To connect " << remote_endpoint.address().to_string() << ":"
            << remote_endpoint.port() << std::endl;
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
        std::cerr << "To write " << buf_to_send.size() << std::endl;
        skinfo->SetSendBuf(buf_to_send);

        // write
        size_t ret = boost::asio::write(skinfo->tcp_sk(),
                                        boost::asio::buffer(skinfo->send_buf()),
                                        boost::asio::transfer_all(),
                                        e);
        if (e) {
            std::cerr << "Write error: " << e.message() << std::endl;
            // shared_ptr 'skinfo' will destruct the SocketInfo
            return -1;
        }

        BOOST_ASSERT(ret == skinfo->send_buf().size());

        // read len_field
        std::cerr << "To read Length_field 4 bytes" << std::endl;
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
        std::cerr << "To read " << len << " bytes" << std::endl;
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

        std::cerr << "To add this socket to map" << std::endl;
        ret = InsertTCPClientSocket(remote_endpoint, skinfo);
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Called when sending request to the server behind and the connection has already established.
    int ToWriteThenReadSync(SocketInfoPtr skinfo, std::vector<char> &buf_to_send,
                        std::vector<char> &buf_to_fill) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To write " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;
        // write
        std::cerr << "To write " << buf_to_send.size() << " bytes" << std::endl;
        skinfo->SetSendBuf(buf_to_send);
        boost::system::error_code e;
        size_t ret = boost::asio::write(skinfo->tcp_sk(),
                                        boost::asio::buffer(skinfo->send_buf()),
                                        boost::asio::transfer_all(),
                                        e);
        if (e) {
            std::cerr << "Write error: " << e.message() << std::endl;
            DestroySocket(skinfo);
            return -1;
        }

        BOOST_ASSERT(ret == skinfo->send_buf().size());

        // read len_field
        std::cerr << "To read Length_field 4 bytes" << std::endl;
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
        std::cerr << "To read " << len << " bytes" << std::endl;
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
    int UDPToWrite(UDPEndpoint to_endpoint, std::vector<char> &buf_to_send) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To write LocalIP:Port <-> RemoteIP:Port "
            << skinfo->local_endpoint().address().to_string() << ":"
            << skinfo->local_endpoint().port() << " <-> " << to_endpoint.address().to_string()
            << ":" << to_endpoint.port() << std::endl;

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
                                                   ret.first,
                                                   boost::asio::placeholders::bytes_transferred));
            udp_socket_->set_in_use(true);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    int UDPToRead() {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        udp_socket_->SetRecvBuf(udp_recv_buf_size_);
        udp_socket_->async_receive_from(boost::asio::buffer(udp_socket_->recv_buf()),
                                       udp_socket_->remote_endpoint(),
                                       boost::bind(&Client::HandleUDPRead,
                                                   shared_from_this(),
                                                   udp_socket_,
                                                   boost::asio::placeholders::bytes_transferred));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

private:
    // Called when beginning to read data from a socket
    int ToReadLThenReadV(SocketInfoPtr skinfo) {

        std::cerr << "Eneter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To read length_field " << skinfo->GetFlow() << std::endl;

        skinfo->SetRecvBuf(4);
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf().at(0)), 4),
            boost::asio::transfer_all(),
                boost::bind(
                    &Server::HandleReadLThenReadV,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    int DestroySocket(SocketInfoPtr skinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To destroy " << skinfo->GetFlow() << std::endl;

        boost::system::error_code e;
        skinfo->tcp_sk().close(e);

        if (skinfo->is_client()) {
            std::map<TCPEndpoint, std::list<SocketInfoPtr> >::iterator it =
                tcp_client_socket_map_.find(skinfo->remote_endpoint());

            if (it == tcp_client_socket_map_.end()) {
                std::cerr << "The SocketInfo to delete doesn't exist." << std::endl;
                std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
                return -1;
            }

            it->second.remove(skinfo);
        } else {
            tcp_server_socket_map_.erase(skinfo->remote_endpoint());
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

private:
    // function for timer handler
    void SweepServerSocket(int milliseconds) {
        PTime expire_time = boost::posix_time::microsec_clock::local_time()
            - boost::posix_time::milliseconds(milliseconds);

        std::map<TCPEndpoint, SocketInfoPtr>::iterator  it, endit;
        it = tcp_server_socket_map_.begin();
        endit = tcp_server_socket_map_.end();
        boost::system::error_code e;
        for (; it != endit;) {
            SocketInfoPtr p = it->second;
            if (p->access_time() < expire_time) {
                tcp_server_socket_map_.erase(it++);
                std::cerr << "This socket is to be desctructed "
                    << p->remote_endpoint().address().to_string() << ":"
                    << p->remote_endpoint().port() << std::endl;
                p->tcp_sk().close(e);
            } else {
                it++;
            }
        } // for
    };

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
        timer_.async_wait(boost::bind(&Server::HandleTimeout,
                                        shared_from_this(),
                                        boost::asio::placeholders::error));
        return;
    };

    // Accept handler
    void HandleAccept(const boost::system::error_code& error, TCPAcceptorInfoPtr acceptorinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        if (error) {
            std::cerr << "Accept error: " << error.message() << std::endl;
            exit(1);
            return;
        }

        SocketInfoPtr sk_info = acceptorinfo->sk_info();
        sk_info->set_is_connected(true);
        sk_info->set_in_use(true);
        acceptorinfo->Reset();
        boost::system::error_code e;
        TCPEndpoint tmp_endpoint = sk_info->tcp_sk().remote_endpoint(e);
        if (e) {
            std::cerr << " Get remote_endpoint of the socket error: " << e.message() << std::endl;
            return;
        }
        TCPEndpoint tmp_endpoint2 = sk_info->tcp_sk().local_endpoint(e);
        if (e) {
            std::cerr << " Get local_endpoint of the socket error: " << e.message() << std::endl;
            return;
        }

        sk_info->set_remote_endpoint(tmp_endpoint);
        sk_info->set_local_endpoint(tmp_endpoint2);

        std::cerr << "A socket is accepted " << sk_info->GetFlow() << std::endl;

        std::cerr << "Add the socket to map" << std::endl;
        int ret = InsertTCPServerSocket(tmp_endpoint, sk_info);
        if (ret < 0) {
            std::cerr << "Insert server socket error: " << ret << std::endl;
            exit(1);
        }

        boost::system::error_code read_ec;
        sk_info->SetRecvBuf(init_recv_buf_size_);

        if (ToReadLThenReadV(sk_info) < 0) {
            DestroySocket(sk_info);
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        std::cerr << "Go on accepting "  << std::endl;

        acceptorinfo->acceptor().async_accept(acceptorinfo->sk_info()->tcp_sk(),
                boost::bind(
                    &Server::HandleAccept,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    acceptorinfo));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;

    };

    // Read handlers
    void HandleReadLThenReadV(const boost::system::error_code& error,
                            SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        if (error) {
            std::cerr << "Read error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            return;
        }


        int len = *(int*)(&((skinfo->recv_buf()[0])));
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        std::cerr << "Length read:  " << len << skinfo->GetFlow() << std::endl;

        if (len > max_recv_buf_size_) {

            std::cerr << "Data length is too large, so close: " << len << ">" << max_recv_buf_size_ << std::endl;
            DestroySocket(skinfo);
            return;
        }

        if (len > static_cast<int>(skinfo->recv_buf().size())) {
            skinfo->SetRecvBuf(len);
            *(int*)(&skinfo->recv_buf()[0]) = boost::asio::detail::socket_ops::network_to_host_long(len);
        }

        std::cerr << "To read " << len << " bytes" << std::endl;
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf()[4]), len - 4),
            boost::asio::transfer_all(),
                boost::bind(
                    &Server::HandleReadVThenProcess,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;


    };

    void HandleReadVThenProcess(const boost::system::error_code& error,
                               SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        boost::system::error_code e;
        std::cerr << "To process: " << skinfo->recv_buf().size() << " bytes from "<< skinfo->GetFlow() << std::endl;

        if (error) {
            std::cerr << "ReadV error: " << error.message() << std::endl;
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        int len = *(int*)(&((skinfo->recv_buf().at(0))));
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        BOOST_ASSERT(static_cast<int>(byte_num) == len - 4);

        std::string fromip, toip;
        fromip = skinfo->remote_endpoint().address().to_string();
        toip = skinfo->local_endpoint().address().to_string();

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
            // this is a server socket, and to idle
            BOOST_ASSERT(!skinfo->is_client());
        } else if (ret == 1) {
            // this is a client socket, send and idle
            BOOST_ASSERT(skinfo->is_client());
            BOOST_ASSERT(skinfo->in_use());
            skinfo->set_in_use(false);
        } else if (ret == 2) {
            // this is a server socket, send and close
            BOOST_ASSERT(skinfo->in_use());
            BOOST_ASSERT(!skinfo->is_client());
        } else if (ret == 3) {
            // this is a server socket, send and then read
            BOOST_ASSERT(!skinfo->is_client());
            ToReadLThenReadV(skinfo);
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

    // UDP write handler
    void HandleUDPWrite(const boost::system::error_code& error,
                             UDPSocketInfoPtr skinfo,
                             UDPEndpoint remote_endpoint,
                             std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To write " << remote_endpoint.address().to_string() << ":"
            << remote_endpoint.port() << std::endl;

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

    // UDP read handler
    void HandleUDPRead(const boost::system::error_code& error,
                             UDPSocketInfoPtr skinfo,
                             std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "Read in " << byte_num << " bytes from LocalIP:Port <-> RemoteIP:Port "
            << skinfo->local_endpoint().address().to_string() << ":"
            << skinfo->local_endpoint().port() << " <-> " << to_endpoint.address().to_string()
            << ":" << to_endpoint.port() << std::endl;

        std::string fromip, toip;
        fromip = skinfo->remote_endpoint().address().to_string();
        toip = skinfo->local_endpoint().address().to_string();

        int ret = ProcessData(skinfo->recv_buf(),
                        fromip,
                        skinfo->remote_endpoint().port(),
                        toip,
                        skinfo->local_endpoint().port(),
                        boost::posix_time::microsec_clock::local_time());

        if (ret < 0) {
            // error
            std::cerr << "ProcessData error" << std::endl;
        } else if (ret == 0) {
            // this is a server socket, and to idle
            BOOST_ASSERT(false);
        } else if (ret == 1) {
            // this is a client socket, send and idle
        } else if (ret == 2) {
            // this is a server socket, send and close
            BOOST_ASSERT(false);
        } else if (ret == 3) {
            // this is a server socket, send and then read
            UDPToRead();
        } else {
            BOOST_ASSERT(false);
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

protected:
    SocketInfoPtr FindTCPServerSocket(TCPEndpoint &key) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To find " << key.address().to_string() << ":" << key.port() << std::endl;

        SocketInfoPtr ret;
        std::map<TCPEndpoint, SocketInfoPtr>::iterator it
            = tcp_server_socket_map_.find(key);

        if (it != tcp_server_socket_map_.end()) {
            ret = it->second;
        }

        if (ret) {
            std::cerr << "Found:" << std::endl;
        } else {
            std::cerr << "Not found:" << std::endl;
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return ret;
    };

    int InsertTCPServerSocket(TCPEndpoint &key, SocketInfoPtr new_skinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To insert " << key.address().to_string() << ":" << key.port() << std::endl;

        std::map<TCPEndpoint, SocketInfoPtr>::iterator it
            = tcp_server_socket_map_.find(key);

        if (it != tcp_server_socket_map_.end()) {
            std::cerr << "This server socket already exists(cann't be)" << std::endl;
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }

        std::pair<std::map<TCPEndpoint, SocketInfoPtr>::iterator, bool> insert_result =
            tcp_server_socket_map_.insert(std::pair<TCPEndpoint, SocketInfoPtr>(key, new_skinfo));
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    int InsertTCPAcceptor(TCPEndpoint &key, TCPAcceptorInfoPtr new_acceptorinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To insert " << key.address().to_string() << ":" << key.port() << std::endl;
        std::map<TCPEndpoint, TCPAcceptorInfoPtr>::iterator it
            = tcp_acceptor_map_.find(key);

        if (it != tcp_acceptor_map_.end()) {
            std::cerr << "This server socket already exists(cann't be)" << std::endl;
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }

        std::pair<std::map<TCPEndpoint, TCPAcceptorInfoPtr>::iterator, bool> insert_result =
            tcp_acceptor_map_.insert(std::pair<TCPEndpoint, TCPAcceptorInfoPtr>(key, new_acceptorinfo));
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    SocketInfoPtr FindIdleTCPClientSocket(TCPEndpoint &key) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To find " << key.address().to_string() << ":" << key.port() << std::endl;

        SocketInfoPtr ret;
        std::map<TCPEndpoint, std::list<SocketInfoPtr> >::iterator it
            = tcp_client_socket_map_.find(key);

        if (it != tcp_client_socket_map_.end()) {
            std::list<SocketInfoPtr>::iterator list_it = it->second.begin();
            while (list_it != it->second.end()) {
                if ((*list_it)->in_use() || (!(*list_it)->is_connected())) {
                    list_it++;
                } else {
                    ret = *list_it;
                    break;
                }
            } // while
        }
        if (ret) {
            std::cerr << "Found" << std::endl;
        } else {
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
            it->second.push_back(new_skinfo);
        } else {
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
    int tcp_backlog() { return tcp_backlog_; };
    void set_tcp_backlog(int backlog) { tcp_backlog_ = backlog; };
    int max_recv_buf_size() { return max_recv_buf_size_; };
    void set_max_recv_buf_size(int max_recv_buf_size) { max_recv_buf_size_ = max_recv_buf_size; };
    int init_recv_buf_size(){ return init_recv_buf_size_; };
    void set_init_recv_buf_size(int init_recv_buf_size) { init_recv_buf_size_ = init_recv_buf_size; };
    void set_timer_trigger_interval(int millisec) { timer_trigger_interval_ = millisec; };
    int timer_trigger_interval() { return timer_trigger_interval_; };
    int server_timeout() { return server_timeout_; };
    void set_server_timeout(int server_timeout) { server_timeout_ = server_timeout; };
    void set_udp_recv_buf_size(int udp_recv_buf_size) { udp_recv_buf_size_ = udp_recv_buf_size;};
    int udp_recv_buf_size() { return udp_recv_buf_size_;};
private:
    // io_service
    boost::asio::io_service io_serv_;
    // Data structure to maintain sockets(connections)
    UDPSocketInfoPtr udp_socket_;
    std::map<TCPEndpoint, SocketInfoPtr> tcp_server_socket_map_;
    std::map<TCPEndpoint, TCPAcceptorInfoPtr> tcp_acceptor_map_;
    std::map<TCPEndpoint, std::list<SocketInfoPtr> > tcp_client_socket_map_;

    // timer
    int timer_trigger_interval_;
    boost::asio::deadline_timer timer_;

    // timer handlers
    std::list<boost::function<void()> > timer_handler_list_;
    // socket parameters
    int init_recv_buf_size_;
    int max_recv_buf_size_;
    int udp_recv_buf_size_;
    int tcp_backlog_;
    int server_timeout_;
};

};// namespace

#endif
