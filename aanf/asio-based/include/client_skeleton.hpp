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

#ifndef _CLIENT_SKELETON_HPP_
#define _CLIENT_SKELETON_HPP_

#include <boost/asio.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include <string>
#include <map>
#include <list>

#include "basic_connection.hpp"
#include "connection_pool.hpp"

namespace AANF {

class ClientSkeleton {

public:
    typedef boost::asio::tcp::acceptor Acceptor;
    typedef boost::asio::io_service IO_Service;
    typedef boost::asio::ip::tcp::endpoint TCP_Endpoint;
    typedef boost::asio::ip::address IP_Address;
    typedef boost::asio::ip::tcp::socket TCP_Socket;

public:
    ClientSkeleton(int thread_pool_size = 4)
        :strand_(io_service_),
        thread_pool_size_(thread_pool_size) {
    };


    void Run() {
        std::vector<boost::shared_ptr<boost::thread> > threads;
        for (std::size_t i = 0; i <　thread_pool_size; i++) {
            boost::shared_ptr<boost::thread> thread(
                new boost::thread(boost::bind(&ClientSkeleton::ThreadProc, this)));
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
    void ThreadProc() {

        boost::mt19937 gen;
        boost::uniform_int<> dist(1, 1000000);
        boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(gen, dist);

        while (true) {
            boost::shared_ptr<MessageInfo> msg_to_send = PrepareRequest(die());

            if (!msg_to_send) {
                continue;
            }

            // 得到connection
            ConnectionKey key;
            key.addr_ = boost::asio::ip::address::from_string(msg_to_send->to_ip());
            key.port_ = msg_to_send->to_port();

            boost::shared_ptr<BasicConnection> connection_to_use =
                ConnectionPool::GetConnectionPool()->GetIdleConnection(key);

            boost::system::error_code ec;
            if (!connection_to_use) {
                connection_to_use = CreateConnection(0, msg_to_send->my_ip(), msg_to_send->my_port());
                ConnectionPool::GetConnectionPool()->InsertConnection(key, connection_to_use);
                connection_to_use->StartConnect(ec, msg_to_send->to_ip(), msg_to_send->to_port(), msg_to_send);
            } else {
                connection_to_use->StartConnect(ec, msg_to_send->to_ip(), msg_to_send->to_port(), msg_to_send);
            }
        } // while

    };

    boost::shared_ptr<MessageInfo> PrepareRequest(int type) = 0;

private:
    IO_Service io_service_;
    boost::asio::io_service::strand strand_;
    int thread_pool_size_;


};
};
#endif
