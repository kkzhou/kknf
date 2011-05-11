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

typedef (boost::shared_ptr<BasicConnect>)(*ConnectionFactory)();

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
    boost::shared_ptr<BasicConnection> CreateConnection(int type, std::string &my_ip, uint16_t my_port) {

        boost::system::error_code ec;
        IP_Address my_addr = boost::asio::ip::address::from_string(my_ip, ec);
        if (ec == boost::system::errc.bad_address) {
            return -1;
        }

        TCP_Endpoint my_endpoint(my_addr, my_port);
        boost::shared_ptr<BasicConnect> new_connection(new BinConnection(io_service_, 1024, 10240));
        new_connection->socket()->bind(my_endpoint);
        return new_connection;
    };

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
                connection_to_use->StartWrite(ec, msg_to_send);
            }
        } // while

    };
    boost::shared_ptr<MessageInfo> PrepareRequest(int type) = 0;
    void Stop() {
    };

private:
    void HandleAccept(boost::system::error_code &ec, Acceptor &acceptor,
                boost::shared_ptr<BasicConnection> connection) {

        if (!ec) {

            connection->Use();
            connection->start();
            boost::shared_ptr<BasicConnection> new_connection(ConnectionFactory());

            acceptor.async_accept(new_connection->socket(),
                strand_.wrap(
                    boost::bind(
                        ServerSkeleton::HandleAccept,
                        this, boost::asio::placeholders::error,
                        acceptor, ConnectionFactory()
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
