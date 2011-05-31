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
    Client()
        : init_recv_buf_size_(1024),
        max_recv_buf_size_(1024 * 1024 * 2) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    virtual ~Client(){
        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
    };

    // Initiate a UDP socket, one is enough
    int InitUDPSocket(UDPEndpoint local_endpoint) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        udp_socket_.reset(new UDPSocketInfo(io_serv_));
        boost::system::error_code e;
        udp_socket_->udp_sk().bind(local_endpoint, e);

        if (e) {
            std::cerr << "Bind UDP socket error: " << e.message() << std::endl;
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }
        udp_socket_->set_local_endpoint(local_endpoint);
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

public:
   // Sync interfaces
    // Called when sending request to the server behind and the connection hasn't established yet.
    int ToWriteThenReadSync(TCPEndpoint &remote_endpoint, std::vector<char> &buf_to_send/*content swap*/,
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
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }

        skinfo->set_is_connected(true);
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
            // shared_ptr 'skinfo' will destruct the SocketInfo
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
            // shared_ptr 'skinfo' will destruct the SocketInfo
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }
        BOOST_ASSERT(ret == 4);

        len = *reinterpret_cast<int*>(&len_field[0]);
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        if (len < 0 || len > max_recv_buf_size()) {
            std::cerr << "len_field invalid: " << len << std::endl;
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
            // shared_ptr 'skinfo' will destruct the SocketInfo
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -3;
        }

        std::cerr << "To add this socket to map" << std::endl;
        ret = InsertTCPClientSocket(remote_endpoint, skinfo);
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        return 0;
    };

    // Called when sending request to the server behind and the connection has already established.
    int ToWriteThenReadSync(SocketInfoPtr skinfo, std::vector<char> &buf_to_send/*content swap*/,
                        std::vector<char> &buf_to_fill) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;
        std::cerr << "To write " << skinfo->remote_endpoint().address().to_string() << ":"
            << skinfo->remote_endpoint().port() << std::endl;
        // write
        std::cerr << "To write " << buf_to_send.size() << " bytes" << std::endl;

        skinfo->SetSendBuf(buf_to_send);
        boost::system::error_code e;
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
        std::cerr << "To read " << len << " bytes" << std::endl;

        if (len < 0 || len > max_recv_buf_size()) {
            std::cerr << "len_field invalid: " << len << std::endl;
            DestroySocket(skinfo);
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -2;
        }

        buf_to_fill.resize(len);
        std::copy(len_field.begin(), len_field.end(), buf_to_fill.begin());

        ret = boost::asio::read(
                skinfo->tcp_sk(),
                boost::asio::buffer(&buf_to_fill[4], len - 4),
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
    // To write data into the UDP socket
    int UDPToWriteThenReadSync(UDPEndpoint to_endpoint, std::vector<char> &buf_to_send,
                               std::vector<char> &buf_to_fill, UDPEndpoint &from_endpoint) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << std::endl;

        std::cerr << "To write LocalIP:Port <-> RemoteIP:Port "
            << udp_socket_->local_endpoint().address().to_string() << ":"
            << udp_socket_->local_endpoint().port() << " <-> " << to_endpoint.address().to_string()
            << ":" << to_endpoint.port() << std::endl;

        std::cerr << "To write " << buf_to_send.size() << " bytes" << std::endl;
        udp_socket_->udp_sk().send_to(boost::asio::buffer(buf_to_send), to_endpoint, 0, e);

        if (e) {

            std::cerr << "Write error: " << e.message() << std::endl;
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }

        static std::vector<char> recv_buf(65535);

        std::size_t ret =
            udp_socket_->udp_sk().receive_from(boost::asio::buffer(recv_buf), from_endpoint, 0, e);

        if (e) {

            std::cerr << "Read error: " << e.message() << std::endl;
            std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
            return -1;
        }

        buf_to_fill.reserve(ret);
        std::copy(recv_buf.begin(), recv_buf.begin() + ret, buf_to_fill.begin());
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
                std::cerr << "Fail " << __FUNCTION__ << ":" << __LINE__ << std::endl;
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
protected:
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
    int max_recv_buf_size() { return max_recv_buf_size_; };
    void set_max_recv_buf_size(int max_recv_buf_size) { max_recv_buf_size_ = max_recv_buf_size; };
    int init_recv_buf_size(){ return init_recv_buf_size_; };
    void set_init_recv_buf_size(int init_recv_buf_size) { init_recv_buf_size_ = init_recv_buf_size; };
private:
    // io_service
    boost::asio::io_service io_serv_;
    // Data structure to maintain sockets(connections)
    UDPSocketInfoPtr udp_socket_;
    std::map<TCPEndpoint, std::list<SocketInfoPtr> > tcp_client_socket_map_;
    // socket parameters
    int init_recv_buf_size_;
    int max_recv_buf_size_;
};

};// namespace

#endif
