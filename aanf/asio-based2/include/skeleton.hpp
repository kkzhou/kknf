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

#ifndef _SKELETON_HPP_
#define _SKELETON_HPP_

#include <stdlib.h>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
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

// The skeleton of a server/client
// Support event-driven mode, thread-pool mode, sync mode
class Skeleton : public boost::enable_shared_from_this<Skeleton> {

public:
    // The only interface should be overrided in derived classes to deal with business logic
    // Process data
    // return value:
    // <0:  error happened
    // 0:  this is a server socket, to set idle
    // 1:  this is a client socket, to set idle
    // 2:  this is a server socket, to close
    // 3:  this is a server socket, to read
    virtual int ProcessData(std::vector<char> &input_data, std::string &from_ip, uint16_t from_port,
                    std::string &to_ip, uint16_t to_port, PTime arrive_time) {
        BOOST_ASSERT(false);
        return 0;
    };

public:
    Skeleton()
        : strand_(io_serv_),
        timer_trigger_interval_(10000),
        timer_(io_serv_),
        init_recv_buf_size_(1024),
        max_recv_buf_size_(1024 * 1024 * 2),
        tcp_backlog_(1024),
        server_timeout_(10000) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;

    };

    virtual ~Skeleton(){

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        // TODO: release system resources
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    int InitUDPSocket(UDPEndpoint local_endpoint) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        udp_socket_.reset(new UDPSocketInfo(io_serv_));
        boost::system::error_code e;
        udp_socket_->udp_sk().bind(local_endpoint, e);

        if (e) {
            std::cerr << "Bind UDP socket error: " << e.message() << std::endl;
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }
        udp_socket_->set_local_endpoint(local_endpoint);

        udp_socket_->SetRecvBuf(udp_recv_buf_size_);
        udp_socket_->udp_sk().async_receive_from(
                boost::asio::buffer(udp_socket_->recv_buf()),
                udp_socket_->remote_endpoint(),
                strand_.wrap(
                    boost::bind(&Skeleton::HandleUDPRead,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        udp_socket_,
                        boost::asio::placeholders::bytes_transferred)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Register handlers to the timer(only one timer)
    // which means add a handler to a callback chain
    void AddTimerHandler(boost::function<void()> func) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        timer_handler_list_.push_back(func);
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;

    };

    // Add listen socket to accept connections
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
        new_acceptorinfo->acceptor().async_accept(
            new_acceptorinfo->sk_info()->tcp_sk(),
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleAccept,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    new_acceptorinfo)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Start the server in serveral threads.
    void Run() {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        // Add a default timerhandler which is to dealing with
        // idle server sockets
        AddTimerHandler(boost::function<void()>(
            boost::bind(&Skeleton::SweepServerSocket,
                shared_from_this(),
                server_timeout_)));

        timer_.expires_from_now(boost::posix_time::milliseconds(timer_trigger_interval_));
        timer_.async_wait(
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleTimeout,
                    shared_from_this(),
                    boost::asio::placeholders::error)));

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
    int ToWriteThenReadSync(TCPEndpoint &remote_endpoint, std::vector<char> &buf_to_send/*content swap*/,
                        SocketInfo::SocketType type, std::vector<char> &buf_to_fill) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        bool socket_exist = false;

        SocketInfoPtr skinfo = FindIdleTCPClientSocket(remote_endpoint);
        if (skinfo) {
            socket_exist = true;
            BOOST_ASSERT(type == skinfo->type());
        }

        if (type != SocketInfo::T_TCP_LV) {
            std::cerr << "Not supported socket type" << std::endl;
            return -1;
        }

        boost::system::error_code e;
        if (!socket_exist) {

            std::cerr << "To connect " << remote_endpoint.address().to_string() << ":"
                << remote_endpoint.port() << std::endl;

            skinfo = SocketInfoPtr(new SocketInfo(type, io_serv_));

            // connect
            skinfo->set_remote_endpoint(remote_endpoint);
            skinfo->tcp_sk().connect(remote_endpoint, e);

            if (e) {
                std::cerr << "Connect error: " << e.message() << std::endl;
                // shared_ptr 'skinfo' will destruct the SocketInfo
                std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
                return -1;
            }

            skinfo->set_is_connected(true);
            skinfo->set_in_use(true);
            std::cerr << "To add this socket to map" << std::endl;
            InsertTCPClientSocket(remote_endpoint, skinfo);
        }

        std::cerr << "To write " << buf_to_send.size() << std::endl;
        skinfo->SetSendBuf(buf_to_send);

        // write
        size_t ret = boost::asio::write(
                        skinfo->tcp_sk(),
                        boost::asio::buffer(skinfo->send_buf()),
                        boost::asio::transfer_all(),
                        e);
        if (e) {
            std::cerr << "Write error: " << e.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }

        BOOST_ASSERT(ret == skinfo->send_buf().size());

        // read len_field
        std::cerr << "To read Length_field 4 bytes" << std::endl;
        std::vector<char> len_field(4, 0);
        int len = 0;

        ret = boost::asio::read(
                skinfo->tcp_sk(),
                boost::asio::buffer(len_field),
                boost::asio::transfer_all(),
                e);

        if (e) {
            std::cerr << "Read len_field error: " << e.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }
        BOOST_ASSERT(ret == 4);

        len = *reinterpret_cast<int*>(&len_field[0]);
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        if (len < 0 || len > max_recv_buf_size()) {
            std::cerr << "len_field invalid: " << len << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -2;
        }
        std::cerr << "To read " << len << " bytes" << std::endl;
        buf_to_fill.resize(len);
        std::copy(len_field.begin(), len_field.end(), buf_to_fill.begin());
        ret = boost::asio::read(
                skinfo->tcp_sk(),
                boost::asio::buffer(&len_field[4], len - 4),
                boost::asio::transfer_all(),
                e);

        if (e) {
            std::cerr << "Read data_field error: " << e.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -3;
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Async interfaces
    int UDPToWrite(UDPEndpoint to_endpoint, std::vector<char> &buf_to_send/*content swap*/) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To write LocalIP:Port <-> RemoteIP:Port "
            << udp_socket_->local_endpoint().address().to_string() << ":"
            << udp_socket_->local_endpoint().port() << " <-> " << to_endpoint.address().to_string()
            << ":" << to_endpoint.port() << std::endl;

        int ret = udp_socket_->AddSendBuf(to_endpoint, buf_to_send);
        if (ret < 0) {
            std::cerr << "Send buffer list is full " << std::endl;
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }

        if (!udp_socket_->in_use()) {
            // not in sending, so async_send_to

            std::pair<UDPEndpoint, std::vector<char> > front_item =
                udp_socket_->GetSendBufListFront();
            std::cerr << "To write " << front_item.second.size() << " bytes" << std::endl;
            udp_socket_->udp_sk().async_send_to(
                boost::asio::buffer(front_item.second),
                front_item.first,
                strand_.wrap(
                    boost::bind(&Skeleton::HandleUDPWrite,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        udp_socket_,
                        front_item.first,
                        boost::asio::placeholders::bytes_transferred)));

            udp_socket_->set_in_use(true);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    int ToWriteThenCallBack(TCPEndpoint &remote_endpoint, SocketInfo::SocketType type,
                            std::vector<char> &buf_to_send/*content swap*/,
                            PacketCallBack cb) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        SocketInfoPtr skinfo = FindIdleTCPClientSocket(remote_endpoint);

        int ret = 0;
        if (!skinfo) {
            if (i_am_server) {
                std::cerr << "Not found the server socket" << std::endl;
                std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
                return -1;
            }
            ret = ToConnectThenWrite(remote_endpoint, type, buf_to_send, cb);

        } else {

            std::cerr << "To write " << buf_to_send.size() << " bytes "
                << skinfo->GetFlow() << std::endl;
            skinfo->set_cb(cb);
            ret = ToWriteThenRead(skinfo, buf_to_send);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Called when sending request to the server behind and the connection has already established.
    int ToWriteThenRead(TCPEndpoint &remote_endpoint, SocketInfo::SocketType type,
                            std::vector<char> &buf_to_send/*content swap*/, bool i_am_server) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        SocketInfoPtr skinfo;
        if (i_am_server) {
            skinfo = FindTCPServerSocket(remote_endpoint);
        } else {
            skinfo = FindIdleTCPClientSocket(remote_endpoint);
        }

        int ret = 0;
        if (!skinfo) {
            if (i_am_server) {
                std::cerr << "Not found the server socket" << std::endl;
                std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
                return -1;
            }
            PacketCallBack tmp_cb;
            ret = ToConnectThenWrite(remote_endpoint, type, buf_to_send, tmp_cb/*empty*/);

        } else {

            std::cerr << "To write " << buf_to_send.size() << " bytes "
                << skinfo->GetFlow() << std::endl;
            ret = ToWriteThenRead(skinfo, buf_to_send);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return ret;
    };

    int ToWriteThenClose(TCPEndpoint &remote_endpoint,
                         std::vector<char> &buf_to_send/*content swap*/, bool i_am_server) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        SocketInfoPtr skinfo;
        if (i_am_server) {
            skinfo  = FindTCPServerSocket(remote_endpoint);
        } else {
             skinfo = FindIdleTCPClientSocket(remote_endpoint);
        }

        if (!skinfo) {
            std::cerr << "Not found" << skinfo->GetFlow() << std::endl;
            return -1;
        }

        std::cerr << "To write " << buf_to_send.size() << " bytes " << skinfo->GetFlow() << std::endl;
        skinfo->SetSendBuf(buf_to_send);

        boost::asio::async_write(
            skinfo->tcp_sk(),
            boost::asio::buffer(skinfo->send_buf()),
            boost::asio::transfer_all(),
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleWriteThenClose,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

private:
   // Called when sending request to the server behind and the connection hasn't established yet.
    int ToConnectThenWrite(TCPEndpoint &remote_endpoint, SocketInfo::SocketType type,
                            std::vector<char> &buf_to_send/*content swap*/, PacketCallBack cb) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To connect " << remote_endpoint.address().to_string() << ":"
            << remote_endpoint.port() << std::endl;

        SocketInfoPtr skinfo(new SocketInfo(type, io_serv_));
        skinfo->set_cb(cb);
        skinfo->SetSendBuf(buf_to_send);
        skinfo->set_remote_endpoint(remote_endpoint);
        skinfo->set_is_client(true);
        std::cerr << "To add this socket to map" << std::endl;
        InsertTCPClientSocket(remote_endpoint, skinfo);

        skinfo->tcp_sk().async_connect(
            skinfo->remote_endpoint(),
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleConnectThenWrite,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    skinfo->send_buf())));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    int ToWriteThenRead(SocketInfoPtr skinfo, std::vector<char> &buf_to_send/*content swap*/) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        if (skinfo->is_client()) {
            BOOST_ASSERT(!skinfo->in_use());
            skinfo->set_in_use(true);
        }
        std::cerr << "To write " << buf_to_send.size() << " bytes " << skinfo->GetFlow() << std::endl;
        skinfo->SetSendBuf(buf_to_send);    // this will swap

        std::cerr << "Socket type:" << skinfo->type() << std::endl;
        if (skinfo->type() == SocketInfo::T_TCP_HTTP) {
            // this socket handles(read/write) TCP_LV packet
            boost::asio::async_write(
                skinfo->tcp_sk(),
                boost::asio::buffer(skinfo->send_buf()),
                boost::asio::transfer_all(),
                strand_.wrap(
                    boost::bind(
                        &Skeleton::HandleWriteThenReadHTTPHead,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        skinfo,
                        boost::asio::placeholders::bytes_transferred)));

        } else if (skinfo->type() == SocketInfo::T_TCP_LV) {
            // this socket handles(read/write) TCP_LV packet
            boost::asio::async_write(
                skinfo->tcp_sk(),
                boost::asio::buffer(skinfo->send_buf()),
                boost::asio::transfer_all(),
                strand_.wrap(
                    boost::bind(
                        &Skeleton::HandleWriteThenReadL,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        skinfo,
                        boost::asio::placeholders::bytes_transferred)));

        } else if (skinfo->type() == SocketInfo::T_TCP_TLV) {
            // this socket handles(read/write) TCP_TLV packet
            std::cerr << "To be supported socket type:" << skinfo->type() << std::endl;
            BOOST_ASSERT(false);
        } else if (skinfo->type() == SocketInfo::T_TCP_LINE) {
            // this socket handles(read/write) TCP_LINE packet
            std::cerr << "To be supported socket type:" << skinfo->type() << std::endl;
            BOOST_ASSERT(false);
        } else {
            std::cerr << "Not supported socket type:" << skinfo->type() << std::endl;
            BOOST_ASSERT(false);
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };


    std::size_t HTTPHeadReadCompletionCondition(const boost::system::error_code &error,
                                                SocketInfoPtr skinfo, std::size_t byte_num) {

        if (error) {
            std::cerr << "Read error: " << error.message() << std::endl;
            return 0;
        }

        std::vector<char>::iterator it;
        char key[] = "\r\n\r\n";
        std::cerr << byte_num << " bytes comes, already recveived "
            << skinfo->recv_buf_filled() << " total recv buf is "
            << skinfo->recv_buf().size() << std::endl;
        skinfo->set_recv_buf_filled(skinfo->recv_buf_filled() + byte_num);

        it = std::search(skinfo->recv_buf().begin(), skinfo->recv_buf().end(), key, key + 4);
        if (it != skinfo->recv_buf().end()) {
            return 0;
        }

        if (skinfo->recv_buf().size() == skinfo->recv_buf_filled()) {

            std::cerr << "Read HTTP Head error: too large head: "
                << skinfo->recv_buf_filled() << std::endl;
            return 0;
        }

        return skinfo->recv_buf().size() - skinfo->recv_buf_filled();
    };

    int ToReadHTTPHeadThenReadHTTPContent(SocketInfoPtr skinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To read HTTPHead from " << skinfo->GetFlow() << std::endl;

        skinfo->SetRecvBuf(init_recv_buf_size_);

        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(skinfo->recv_buf()),
            boost::bind(
                &Skeleton::HTTPHeadReadCompletionCondition,
                shared_from_this(),
                boost::asio::placeholders::error,
                skinfo,
                boost::asio::placeholders::bytes_transferred),
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleReadHTTPHeadThenReadHTTPContent,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    int ToReadLThenReadV(SocketInfoPtr skinfo) {

        std::cerr << "Eneter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To read length_field " << skinfo->GetFlow() << std::endl;

        skinfo->SetRecvBuf(4);
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf().at(0)), 4),
            boost::asio::transfer_all(),
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleReadLThenReadV,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));

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
                std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
                return -1;
            }

            it->second.remove(skinfo);
        } else {
            tcp_server_socket_map_.erase(skinfo->remote_endpoint());
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    int UDPToRead() {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        udp_socket_->SetRecvBuf(udp_recv_buf_size_);
        udp_socket_->udp_sk().async_receive_from(
            boost::asio::buffer(udp_socket_->recv_buf()),
            udp_socket_->remote_endpoint(),
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleUDPRead,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    udp_socket_,
                    boost::asio::placeholders::bytes_transferred)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

private:
    // handler placeholder
    void HandleAnythingThenNothing(const boost::system::error_code& error,
                               SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        if (error) {
            std::cerr << "Do things error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }
        skinfo->Touch();
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

    // function for timer handler
    void SweepServerSocket(int milliseconds) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

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
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

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

        std::cerr << "Reload timer" << std::endl;
        timer_.expires_from_now(boost::posix_time::milliseconds(timer_trigger_interval_));
        timer_.async_wait(
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleTimeout,
                    shared_from_this(),
                    boost::asio::placeholders::error)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
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
        sk_info->set_type(acceptorinfo->type());
        acceptorinfo->Reset();
        boost::system::error_code e;
        TCPEndpoint tmp_endpoint = sk_info->tcp_sk().remote_endpoint(e);

        if (e) {
            std::cerr << " Get remote_endpoint of the socket error: " << e.message() << std::endl;
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        TCPEndpoint tmp_endpoint2 = sk_info->tcp_sk().local_endpoint(e);
        if (e) {
            std::cerr << " Get local_endpoint of the socket error: " << e.message() << std::endl;
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        sk_info->set_remote_endpoint(tmp_endpoint);
        sk_info->set_local_endpoint(tmp_endpoint2);

        std::cerr << "A socket is accepted " << sk_info->GetFlow() << std::endl;
        std::cerr << "Add the socket to map" << std::endl;
        int ret = InsertTCPServerSocket(tmp_endpoint, sk_info);
        if (ret < 0) {
            std::cerr << "Insert server socket error: " << ret << std::endl;
            BOOST_ASSERT(false);
        }

        boost::system::error_code read_ec;
        sk_info->SetRecvBuf(init_recv_buf_size_);

        if (sk_info->type() == SocketInfo::T_TCP_HTTP) {

            std::cerr << "It's a http connection, to read http head" << std::endl;
            if (ToReadHTTPHeadThenReadHTTPContent(sk_info) < 0) {
                std::cerr << "To read http head error" << std::endl;
                DestroySocket(sk_info);
            }

        } else if (sk_info->type() == SocketInfo::T_TCP_LV) {

            std::cerr << "It's a LV connection, to read Length-field" << std::endl;
            if (ToReadLThenReadV(sk_info) < 0) {
                std::cerr << "To read L field error" << std::endl;
                DestroySocket(sk_info);
            }

        } else {
            std::cerr << "Not supported socket type" << std::endl;
            BOOST_ASSERT(false);
        }


        std::cerr << "Go on accepting "  << std::endl;

        acceptorinfo->acceptor().async_accept(
            acceptorinfo->sk_info()->tcp_sk(),
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleAccept,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    acceptorinfo)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;

    };

    // Connect handlers
    void HandleConnectThenWrite(const boost::system::error_code& error, SocketInfoPtr skinfo,
                                    std::vector<char> &buf_to_send) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        if (error) {
            std::cerr << "Connect error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        skinfo->Touch();

        std::cerr << "Connect OK, then to write " << skinfo->GetFlow() << std::endl;

        boost::system::error_code e;
        TCPEndpoint local_endpoint = skinfo->tcp_sk().local_endpoint(e);

        if (e) {
            std::cerr << "Get local_endpoint error: " << e.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        skinfo->set_local_endpoint(local_endpoint);
        skinfo->set_is_connected(true);
        BOOST_ASSERT(!skinfo->in_use());
        skinfo->set_in_use(true);

        std::cerr << "To write " << buf_to_send.size() << " bytes " << skinfo->GetFlow() << std::endl;
        skinfo->SetSendBuf(buf_to_send);

        std::cerr << "Add the socket to map" << std::endl;
        InsertTCPClientSocket(skinfo->remote_endpoint(), skinfo);

        if (skinfo->type() == SocketInfo::T_TCP_LV) {

            boost::asio::async_write(
                skinfo->tcp_sk(),
                boost::asio::buffer(skinfo->send_buf()),
                boost::asio::transfer_all(),
                strand_.wrap(
                    boost::bind(
                        &Skeleton::HandleWriteThenReadL,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        skinfo,
                        boost::asio::placeholders::bytes_transferred)));

        } else if (skinfo->type() == SocketInfo::T_TCP_LINE) {

            std::cerr << "To be supported socket type" << std::endl;
            BOOST_ASSERT(false);

        } else if (skinfo->type() == SocketInfo::T_TCP_TLV) {

            std::cerr << "To be supported socket type" << std::endl;
            BOOST_ASSERT(false);

        } else if (skinfo->type() == SocketInfo::T_TCP_HTTP) {

            boost::asio::async_write(
                skinfo->tcp_sk(),
                boost::asio::buffer(skinfo->send_buf()),
                boost::asio::transfer_all(),
                strand_.wrap(
                    boost::bind(
                        &Skeleton::HandleWriteThenReadHTTPHead,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        skinfo,
                        boost::asio::placeholders::bytes_transferred)));

        } else {

            std::cerr << "Not supported socket type" << std::endl;
            BOOST_ASSERT(false);

        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };


    void HandleReadVThenProcess(const boost::system::error_code& error,
                               SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        boost::system::error_code e;
        std::cerr << "To process: " << skinfo->recv_buf().size() << " bytes from "<< skinfo->GetFlow() << std::endl;

        if (error) {
            std::cerr << "ReadV error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        skinfo->Touch();
        int len = *(int*)(&((skinfo->recv_buf().at(0))));
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        BOOST_ASSERT(static_cast<int>(byte_num) == len - 4);

        std::string fromip, toip;
        fromip = skinfo->remote_endpoint().address().to_string();
        toip = skinfo->local_endpoint().address().to_string();

        int ret = 0;

        if (skinfo->cb()) {

            ret = (skinfo->cb())(
                    skinfo->recv_buf(),
                    fromip,
                    skinfo->remote_endpoint().port(),
                    toip,
                    skinfo->local_endpoint().port(),
                    boost::posix_time::microsec_clock::local_time());

        } else {

            ret = ProcessData(
                    skinfo->recv_buf(),
                    fromip,
                    skinfo->remote_endpoint().port(),
                    toip,
                    skinfo->local_endpoint().port(),
                    boost::posix_time::microsec_clock::local_time());
        }


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
        } else {
            BOOST_ASSERT(false);
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

    // Write handlers
    void HandleWriteThenReadHTTPHead(const boost::system::error_code& error,
                             SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To read HTTPHead from " << skinfo->GetFlow() << std::endl;

        if (error) {

            std::cerr << "Write error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        skinfo->Touch();

        skinfo->SetRecvBuf(init_recv_buf_size_);
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(skinfo->recv_buf()),
            boost::bind(
                &Skeleton::HTTPHeadReadCompletionCondition,
                shared_from_this(),
                boost::asio::placeholders::error,
                skinfo,
                boost::asio::placeholders::bytes_transferred),
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleReadHTTPHeadThenReadHTTPContent,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

    void HandleReadHTTPHeadThenReadHTTPContent(const boost::system::error_code &error,
                                               SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        if (error) {
            std::cerr << "Read error: " << error.message() << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        skinfo->Touch();
        bool continue_field_found = false;
        std::vector<char>::iterator head_it1, head_it2, head_it3, head_it4;
        char key1[] = "Content-Length ";
        char key3[] = "\r\n\r\n";
        char key4[] = "100-continue";
        int head_len = 0;

        // check "\r\n\r\n"
        head_it3 = std::search(skinfo->recv_buf().begin(),
                          skinfo->recv_buf().end(),
                          key3,
                          key3 + 4);

        if (head_it3 == skinfo->recv_buf().end()) {
            std::cerr << "HTTP head invalid: No \"\r\n\r\n\" found. "
                << std::string(skinfo->recv_buf().begin(), skinfo->recv_buf().end()) << std::endl;
            return;
        }

        head_len = head_it3 - skinfo->recv_buf().begin() + 4;

        // check 'expect 100-continue'
        head_it4 = std::search(skinfo->recv_buf().begin(),
                          skinfo->recv_buf().end(),
                          key4,
                          key4 + 12);

        if (head_it4 != skinfo->recv_buf().end()) {
            std::cerr << "HTTP head contains 'Expect 100-continue'. " << std::endl;
            continue_field_found = true;;
        }

        // to extract content-length
        head_it1 = std::search(skinfo->recv_buf().begin(),
                          skinfo->recv_buf().end(),
                          key1,
                          key1 + 15);

        int len = 0;
        if (head_it1 != skinfo->recv_buf().end()) {
            //extract length_field
            head_it1 += 15;
            head_it2 = std::find(head_it1, skinfo->recv_buf().end(), '\r');
            std::string tmps;
            //tmps.assign(head_it1, head_it2);
            tmps.append(head_it1, head_it2);
            std::cerr << "Content-Length field is " << tmps << std::endl;

            try {
                len = boost::lexical_cast<int>(tmps);
            } catch (boost::bad_lexical_cast &e) {
                std::cerr << "Extract Content-Length(" << tmps <<") error: "
                    << len << " " << e.what() << std::endl;
                std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
                return;
            }

        } else {

            DestroySocket(skinfo);
            std::cerr << "No Content-Length found" << std::endl;
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        std::cerr << "Length read:  " << len << skinfo->GetFlow() << std::endl;

        if (len + head_len > max_recv_buf_size_) {

            std::cerr << "Data length is too large, so close: " << len << ">"
                << max_recv_buf_size_ << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        int bytes_left_to_read = len + head_len - byte_num;

        if (len + head_len > static_cast<int>(skinfo->recv_buf().size())) {
            // realloc recv_buf
            std::vector<char> new_recv_buf;
            new_recv_buf.resize(len + head_len);
            std::copy(skinfo->recv_buf().begin(), skinfo->recv_buf().end(), new_recv_buf.begin());
            skinfo->SetRecvBuf(new_recv_buf);
        }

        std::cerr << "HTTP Head length " << head_len << " bytes" << std::endl;
        std::cerr << "HTTP Content length " << len << " bytes" << std::endl;
        std::cerr << "Has read " << byte_num << " bytes" << std::endl;
        std::cerr << "Left " << bytes_left_to_read << " bytes" << std::endl;

        if (bytes_left_to_read == 0) {
            std::cerr << "Already read the whole request" << std::endl;
            HandleReadHTTPContentThenProcess(error, skinfo, byte_num);

        } else {
            boost::asio::async_read(
                skinfo->tcp_sk(),
                boost::asio::buffer(&(skinfo->recv_buf()[byte_num]),  bytes_left_to_read),
                boost::asio::transfer_all(),
                strand_.wrap(
                    boost::bind(
                        &Skeleton::HandleReadHTTPContentThenProcess,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        skinfo,
                        boost::asio::placeholders::bytes_transferred)));

        }


        // dealing with 100-continue
        if (continue_field_found) {

            static std::string continue_head = "HTTP/1.1 100\r\n\r\n";
            std::vector<char> new_send_buf;
            new_send_buf.resize(continue_head.length());
            std::copy(continue_head.begin(), continue_head.end(), new_send_buf.begin());
            skinfo->SetSendBuf(new_send_buf);

            boost::asio::async_write(
                skinfo->tcp_sk(),
                boost::asio::buffer(skinfo->send_buf()),
                boost::asio::transfer_all(),
                strand_.wrap(
                    boost::bind(
                        &Skeleton::HandleAnythingThenNothing,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        skinfo,
                        boost::asio::placeholders::bytes_transferred)));
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

    void HandleReadHTTPContentThenProcess(const boost::system::error_code& error,
                               SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        if (error) {
            std::cerr << "Read HTTPContent error: " << error.message() << std::endl;
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }
        std::cerr << "To process http request: " << skinfo->recv_buf_filled()
            << " bytes from "<< skinfo->GetFlow() << std::endl;

        // reset the recv state
        skinfo->recv_buf().resize(skinfo->recv_buf_filled());
        skinfo->set_recv_buf_filled(0);

        skinfo->Touch();
        std::string fromip, toip;
        fromip = skinfo->remote_endpoint().address().to_string();
        toip = skinfo->local_endpoint().address().to_string();

        int ret = 0;

        if (skinfo->cb()) {

            ret = (skinfo->cb())(
                    skinfo->recv_buf(),
                    fromip,
                    skinfo->remote_endpoint().port(),
                    toip,
                    skinfo->local_endpoint().port(),
                    boost::posix_time::microsec_clock::local_time());

        } else {

            ret = ProcessData(
                    skinfo->recv_buf(),
                    fromip,
                    skinfo->remote_endpoint().port(),
                    toip,
                    skinfo->local_endpoint().port(),
                    boost::posix_time::microsec_clock::local_time());
        }

        if (ret < 0) {
            // error
            std::cerr << "ProcessData error" << std::endl;
            DestroySocket(skinfo);
        } else if (ret == 0) {
            // this is a server socket, to set idle
            BOOST_ASSERT(!skinfo->is_client());
        } else if (ret == 1) {
            // this is a client socket, to set idle
            BOOST_ASSERT(skinfo->is_client());
            BOOST_ASSERT(skinfo->in_use());
            skinfo->set_in_use(false);
        } else if (ret == 2) {
            // this is a server socket, to close
            BOOST_ASSERT(skinfo->in_use());
            BOOST_ASSERT(!skinfo->is_client());
        } else if (ret == 3) {
            // this is a server socket, to read
            BOOST_ASSERT(!skinfo->is_client());
            ToReadHTTPHeadThenReadHTTPContent(skinfo);

        } else {
            BOOST_ASSERT(false);
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

    void HandleWriteThenReadL(const boost::system::error_code& error,
                             SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To read 4 bytes from " << skinfo->GetFlow() << std::endl;

        if (error) {
            std::cerr << "Read HTTPContent error: " << error.message() << std::endl;
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        skinfo->Touch();
        skinfo->SetRecvBuf(4);
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf()[0]), 4),
            boost::asio::transfer_all(),
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleReadLThenReadV,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));

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
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }
        skinfo->Touch();

        int len = *(int*)(&((skinfo->recv_buf()[0])));
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        std::cerr << "Length read:  " << len << skinfo->GetFlow() << std::endl;

        if (len > max_recv_buf_size_) {

            std::cerr << "Data length is too large, so close: "
                << len << ">" << max_recv_buf_size_ << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return;
        }

        if (len > static_cast<int>(skinfo->recv_buf().size())) {
            skinfo->SetRecvBuf(len);
            *(int*)(&skinfo->recv_buf()[0]) =
                boost::asio::detail::socket_ops::network_to_host_long(len);
        }

        std::cerr << "To read " << len << " bytes" << std::endl;
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&(skinfo->recv_buf()[4]), len - 4),
            boost::asio::transfer_all(),
            strand_.wrap(
                boost::bind(
                    &Skeleton::HandleReadVThenProcess,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    skinfo,
                    boost::asio::placeholders::bytes_transferred)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    void HandleWriteThenClose(const boost::system::error_code& error,
                              SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "Data(" << byte_num << "Bytes) is sent: "
            << skinfo->GetFlow() << std::endl;

        DestroySocket(skinfo);
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

    // UDP write handler
    void HandleUDPWrite(const boost::system::error_code& error, UDPSocketInfoPtr skinfo,
                             UDPEndpoint remote_endpoint, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To write " << remote_endpoint.address().to_string() << ":"
            << remote_endpoint.port() << std::endl;

        if (skinfo->GetSendBufListLength() != 0) {

            std::pair<UDPEndpoint, std::vector<char> > ret =
                udp_socket_->GetSendBufListFront();

            std::cerr << "To write " << ret.second.size() << " bytes" << std::endl;
            udp_socket_->udp_sk().async_send_to(
                boost::asio::buffer(ret.second),
                ret.first,
                strand_.wrap(
                    boost::bind(
                        &Skeleton::HandleUDPWrite,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        udp_socket_,
                        ret.first,
                        boost::asio::placeholders::bytes_transferred)));

        } else {
            BOOST_ASSERT(udp_socket_->in_use());
            udp_socket_->set_in_use(false);
        }

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return;
    };

    // UDP read handler
    void HandleUDPRead(const boost::system::error_code& error,
                             UDPSocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "Read in " << byte_num << " bytes from LocalIP:Port <-> RemoteIP:Port "
            << skinfo->local_endpoint().address().to_string() << ":"
            << skinfo->local_endpoint().port() << " <-> "
            << skinfo->remote_endpoint().address().to_string()
            << ":" << skinfo->remote_endpoint().port() << std::endl;

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
            // this is a server socket, to set idle
            BOOST_ASSERT(false);
        } else if (ret == 1) {
            // this is a client socket, to set idle
        } else if (ret == 2) {
            // this is a server socket, to close
            BOOST_ASSERT(false);
        } else if (ret == 3) {
            // this is a server socket, to read
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
            std::cerr << "Found" << std::endl;
        } else {
            std::cerr << "Not found" << std::endl;
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
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
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
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
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
                tcp_client_socket_map_.insert(
                    std::pair<TCPEndpoint, std::list<SocketInfoPtr> >(key, new_list));

            BOOST_ASSERT(insert_result.second);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

public:
    // setter/getter
    boost::asio::io_service& io_serv() { return io_serv_; };
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
    UDPSocketInfoPtr udp_socket() { return udp_socket_;};
private:
    // io_service
    boost::asio::io_service io_serv_;
    boost::asio::io_service::strand strand_;

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
