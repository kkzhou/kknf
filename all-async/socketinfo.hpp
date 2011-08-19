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
#ifndef _SOCKETINFO_HPP_
#define _SOCKETINFO_HPP_

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>


namespace AANF {

typedef boost::asio::ip::address IPAddress;
typedef boost::asio::ip::tcp::socket TCPSocket;
typedef boost::asio::ip::tcp::endpoint TCPEndpoint;
typedef boost::asio::ip::tcp::acceptor TCPAcceptor;
typedef boost::asio::ip::udp::socket UDPSocket;
typedef boost::asio::ip::udp::endpoint UDPEndpoint;
typedef boost::posix_time::ptime PTime;


class UDPSocketInfo;
typedef boost::shared_ptr<UDPSocketInfo> UDPSocketInfoPtr;

class TCPAcceptorInfo;
class TCPSocketInfo;
typedef boost::shared_ptr<TCPAcceptorInfo> TCPAcceptorInfoPtr;
typedef boost::shared_ptr<TCPSocketInfo> TCPSocketInfoPtr;

typedef boost::function<int(TCPSocketInfoPtr)> TCPSocketCallback;
typedef boost::function<int(UDPSocketInfoPtr)> UDPSocketCallback;
typedef boost::function<int(TCPAcceptorInfoPtr)> TCPAcceptorCallback;

// Simple information about a TCP socket
class TCPSocketInfo {

public:
    TCPSocketInfo(boost::asio::io_service &io_serv)
        : tcp_sk_(io_serv),
        in_use_(false),
        is_client_(false),
        is_connected_(false) {

        access_time_ = create_time_ =
            boost::posix_time::microsec_clock::local_time();

    };

    // setters/getters
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
    TCPSocketCallback cb() { return cb_; };
    void set_cb(TCPSocketCallback cb) {cb_ = cb; };

    void Touch() {
        access_time_ = boost::posix_time::microsec_clock::local_time();
    };

    std::string GetFlow() {

        std::stringstream s;
        s << "(LocalIP:Port <-> RemoteIP:Port) "
            << local_endpoint_.address().to_string() << ":" << local_endpoint_.port() << " <-> "
            << remote_endpoint_.address().to_string() << ":" << remote_endpoint_.port() << std::endl;
        return s.str();
    };

private:
    TCPEndpoint remote_endpoint_;
    TCPEndpoint local_endpoint_;
    TCPSocket tcp_sk_;
    PTime create_time_;
    PTime access_time_;
    bool in_use_;
    bool is_client_;
    bool is_connected_;
    TCPSocketCallback cb_;
};

// Information about a TCP acceptor
class TCPAcceptorInfo {
public:
    TCPAcceptorInfo(boost::asio::io_service &io_serv)
        : acceptor_(io_serv),
        sk_info_(new SocketInfo(io_serv)) {

    };
public:
    TCPAcceptor& acceptor() {
        return acceptor_;
    };

    TCPSocketInfo sk_info() {
        return sk_info_;
    };

    void set_cb(TCPAcceptorCallback cb) { cb_ = cb; };
    TCPAcceptorCallback cb() { return cb_; };

    // Resest means begin to accept the next socket
    // which will be stored in sk_info_
    void Reset() {
        sk_info_.reset(new SocketInfo(type_, acceptor_.get_io_service()));
    };
private:
    TCPAcceptor acceptor_;
    TCPSocketInfo sk_info_; // contains the new socket returned by accept()
    TCPAcceptorCallback cb_;
};

// Simple information about a UDP socket
class UDPSocketInfo {
public:
    UDPSocketInfo(boost::asio::io_service &io_serv)
        : udp_sk_(io_serv),
        in_use_(false) {

    };

    // setters/getters
    UDPEndpoint& local_endpoint() { return local_endpoint_; };
    void set_local_endpoint(UDPEndpoint local_endpoint) { local_endpoint_ = local_endpoint;};
    void set_in_use(bool in_use) { in_use_ = in_use;};
    bool in_use() {return in_use_;};
    UDPSocket& udp_sk() { return udp_sk_; };
    UDPSocketCallback cb() { return cb_; };
    void set_cb(UDPSocketCallback cb) {cb_ = cb; };

private:
    UDPEndpoint local_endpoint_;
    UDPSocket udp_sk_;
    bool in_use_;
    UDPSocketCallback cb_;
};


};//namespace

#endif
