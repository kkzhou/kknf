#ifndef _SERVER_SKELETON_HPP_
#define _SERVER_SKELETON_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <map>
#include <list>
#include "connection.hpp"

namespace AANF {

typedef (boost::shared_ptr<BasicConnect>)(*ConnectionFactory)();

class ServerSkeleton {

public:
    typedef boost::asio::tcp::acceptor Acceptor;
    typedef boost::asio::io_service IO_Service;
    typedef boost::asio::ip::tcp::endpoint TCP_Endpoint;
    typedef boost::asio::ip::address IP_Address;
    typedef boost::asio::ip::tcp::socket TCP_Socket;

public:
    ServerSkeleton() 
        :strand_(io_service_) {
    };
        
    int AddListenSocket(std::string &ip, uint16_t port, int backlog, 
            ConnectionFactory factory) {

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

        return 0;
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


};
};
#endif
