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
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>


namespace AANF {

typedef boost::asio::ip::address IPAddress;
typedef boost::asio::ip::tcp::socket TCPSocket;
typedef boost::shared_ptr<TCPSocket> TCPSocketPtr;
typedef boost::asio::ip::tcp::endpoint TCPEndpoint;
typedef boost::asio::ip::tcp::acceptor TCPAcceptor;
typedef boost::asio::ip::udp::socket UDPSocket;
typedef boost::shared_ptr<UDPSocket> UDPSocketPtr;
typedef boost::asio::ip::udp::endpoint UDPEndpoint;
typedef boost::posix_time::ptime PTime;


class UDPSocketInfo;
typedef boost::shared_ptr<UDPSocketInfo> UDPSocketInfoPtr;

class TCPAcceptorInfo;
class SocketInfo;
typedef boost::shared_ptr<TCPAcceptorInfo> TCPAcceptorInfoPtr;
typedef boost::shared_ptr<SocketInfo> SocketInfoPtr;

// Simple information about a TCP socket
class SocketInfo {
public:
    enum SocketType {
        T_TCP_LINE,     // '\n' is the terminator of a packet
        T_TCP_LV,       // the first 4 bytes is an 'int' field to indicate the length
        T_TCP_TLV,      // the first 4 bytes indicates 'type' and the second 4 bytes indicates 'length'
        T_TCP_HTTP,     // http packet
        T_TCP_USR1,
        T_TCP_UNKNOWN
    };

public:
    SocketInfo(SocketType type, boost::asio::io_service &io_serv)
        : type_(type),
        tcp_sk_(io_serv),
        in_use_(false),
        is_client_(false),
        is_connected_(false) {

        access_time_ = create_time_ =
            boost::posix_time::microsec_clock::local_time();

    };

    // setters/getters
    SocketType type() { return type_; };
    void set_type(SocketInfo::SocketType type) { type_ = type; };
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
    void set_recv_buf_filled(std::size_t filled) { recv_buf_filled_ = filled; };
    std::size_t recv_buf_filled() { return recv_buf_filled_; };
    std::vector<char>& send_buf() { return send_buf_; };


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

    void SetSendBuf(std::vector<char> &to_send) {
        send_buf_.swap(to_send);
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
    SocketType type_;
    TCPSocket tcp_sk_;
    PTime create_time_;
    PTime access_time_;
    bool in_use_;
    bool is_client_;
    bool is_connected_;
    std::vector<char> recv_buf_;
    std::size_t recv_buf_filled_;
    std::vector<char> send_buf_;

};

// Information about a TCP acceptor
class TCPAcceptorInfo {
public:
    TCPAcceptorInfo(SocketInfo::SocketType type, boost::asio::io_service &io_serv)
        : type_(type),
        acceptor_(io_serv),
        sk_info_(new SocketInfo(type_, io_serv)) {

    };
public:
    TCPAcceptor& acceptor() {
        return acceptor_;
    };

    SocketInfo::SocketType type() {
        return type_;
    };

    SocketInfoPtr sk_info() {
        return sk_info_;
    };

    // Resest means begin to accept the next socket
    // which will be stored in sk_info_
    void Reset() {
        sk_info_.reset(new SocketInfo(type_, acceptor_.get_io_service()));
    };
private:
    SocketInfo::SocketType type_;
    TCPAcceptor acceptor_;
    SocketInfoPtr sk_info_; // contains the new socket returned by accept()
};



// Simple information about a UDP socket
class UDPSocketInfo {
public:
    UDPSocketInfo(boost::asio::io_service &io_serv)
        : max_length_of_send_buf_list_(100),
        udp_sk_(io_serv),
        in_use_(false) {

    };

    // setters/getters
    UDPEndpoint& local_endpoint() { return local_endpoint_; };
    void set_local_endpoint(UDPEndpoint local_endpoint) { local_endpoint_ = local_endpoint;};
    UDPEndpoint& remote_endpoint() { return remote_endpoint_; };
    void set_remote_endpoint(UDPEndpoint remote_endpoint) { remote_endpoint_ = remote_endpoint;};
    void set_in_use(bool in_use) { in_use_ = in_use;};
    bool in_use() {return in_use_;};
    UDPSocket& udp_sk() { return udp_sk_; };
    std::vector<char>& recv_buf() { return recv_buf_; };
    void set_max_length_of_send_buf_list(size_t max) {max_length_of_send_buf_list_ = max;};

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
    size_t max_length_of_send_buf_list_;
    UDPEndpoint local_endpoint_;
    UDPEndpoint remote_endpoint_;
    UDPSocket udp_sk_;
    bool in_use_;
    std::vector<char> recv_buf_;
    std::list<std::pair<UDPEndpoint, std::vector<char> > > send_buf_list_;
};


};//namespace

#endif
