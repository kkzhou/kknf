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

#ifndef _SERVER_SKELETON_HPP_
#define _SERVER_SKELETON_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>

#include <string>
#include <map>
#include <list>

#include "basic_connection.hpp"

namespace AANF {

class ServerSkeleton {

public:
    typedef boost::asio::tcp::acceptor Acceptor;
    typedef boost::asio::io_service IO_Service;
    typedef boost::asio::ip::tcp::endpoint TCP_Endpoint;
    typedef boost::asio::ip::address IP_Address;
    typedef boost::asio::ip::tcp::socket TCP_Socket;

public:
    ServerSkeleton(int thread_pool_size = 4)
        :strand_(io_service_),
        thread_pool_size_(thread_pool_size) {
    };

    boost::asio::io_service& io_service() {return io_service_;};

    int AddListenSocket(std::string &ip, uint16_t port, int backlog,
                        boost::functor<BasicConnection*()> factory) {

        boost::system::error_code ec;
        IP_Address addr = boost::asio::ip::address::from_string(ip, ec);
        if (ec == boost::system::errc.bad_address) {
            return -1;
        }

        TCP_Endpoint endpoint(addr, port);
        Acceptor accpetor(endpoint);
        acceptor.open(endpoint.protocol());
        acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor.bind(endpoint);
        acceptor.listen(backlog);

        boost::shared_ptr<BasicConnection> new_connection(factory());

        acceptor.async_accept(new_connection->socket(),
            strand_.wrap(
                boost::bind(
                    ServerSkeleton::HandleAccept,
                    this, boost::asio::placeholders::error,
                    acceptor, new_connection, factory
                )
            )
        );

        return 0;
    };

    void Run() {
        std::vector<boost::shared_ptr<boost::thread> > threads;
        for (std::size_t i = 0; i <　thread_pool_size; i++) {
            boost::shared_ptr<boost::thread> thread(
                new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_)));
            threads.push_back(thread);
        }
        for (std::size_t i = 0; i < thread_pool_size_; i++) {
            threads[i]->join();
        }
    };
    void Stop() {
        io_service_.stop();
    };

private:
    void HandleAccept(boost::system::error_code &ec, Acceptor &acceptor,
                boost::shared_ptr<BasicConnection> connection, boost::functor<BasicConnection*()> factory) {

        if (!ec) {

            connection->Use();

            ConnectionKey key;
            key.addr_ = boost::asio::ip::address::from_string(new_connection->socket()->remote_point()->address());
            key.port_ = (new_connection->socket()->remote_point()->port());

            ConnectionPool::GetConnectionPool()->InsertConnection(key, connection);
            boost::system::error_code read_ec;
            connection->StartRead(read_ec);
            boost::shared_ptr<BasicConnection> new_connection(factory());

            acceptor.async_accept(new_connection->socket(),
                strand_.wrap(
                    boost::bind(
                        ServerSkeleton::HandleAccept,
                        this, boost::asio::placeholders::error,
                        acceptor, new_connection, factory
                    )
                )
            );

        } else {
        }
    };

private:
    IO_Service io_service_;
    boost::asio::io_service::strand strand_;
    int thread_pool_size_;
};
};
#endif
