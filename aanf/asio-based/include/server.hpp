#include <boost/asio/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <map>
#include <list>
#include <vector>


namespace AANF {




// The key for a socket used in std::map
class SocketKey {
public:
    SocketKey(std::string &ip, uint16_t port)
        : ip_(ip),
        port_(port) {};

    std::string& ip() {return ip_;};
    uint16_t port() {return port_;};
private:
    string ip_;
    uint16_t port_;
};

enum SocketType {
    T_UDP,
    T_TCP_LINE,
    T_TCP_LV,
    T_TCP_TLV,
    T_TCP_HTTP,
    T_TCP_USR1,
    T_TCP_USR2,
    T_TCP_USR3,
    T_TCP_USR4,
    T_TCP_USR5,
    T_TCP_USR6,
    T_TCP_UNKNOWN
};
// Simple information about a socket
class SocketInfo {
public:
    SocketInfo(SocketType type, boost::asio::io_service &io_serv)
        : type_(type),
        tcp_sk_(io_serv),
        udp_sk_(io_serv) {

        access_time_ = create_time_ =
            boost::posix_time::second_time::localtime();
    };
public:
    SocketType type_;
    TCPSocket tcp_sk_;
    UDPSocket udp_sk_;
    PTime create_time_;
    PTime access_time_;
};

// Information about an acceptor
class TCPAcceptorInfo {
public:
    TCPAcceptorInfo(SocketType type, TCPEndpoint &local_endpoint)
        : type_(type),
        acceptor_(local_endpoint),
        new_socket_(0) {
    };
public:
    TCPAcceptor acceptor_;
    SocketType type_;
    TCPScoketPtr new_socket_;
};

typedef boost::asio::ip::udp::socket UDPSocket;
typedef boost::shared_ptr<UDPSocket> UDPSocketPtr;
typedef boost::asio::ip::udp::endpoint UDPEndpoint;

typedef boost::asio::ip::tcp::socket TCPSocket;
typedef boost::shared_ptr<TCPSocket> TCPSocketPtr;
typedef boost::asio::ip::tcp::endpoint TCPEndpoint;
typedef boost::asio::ip::tcp::acceptor TCPAcceptor;
typedef boost::shared_ptr<TCPAcceptorInfo> TCPAcceptorInfoPtr;

typedef boost::shared_ptr<SocketInfo> SocketInfoPtr;

typedef boost::date_time::posix_time PTime;


class Server : public enable_shared_from_this<ServerSkeleton> {
public:
    Server()
        : strand_(io_serv_) {
    };
    int AddTCPListenSocket(std::string &ip, uint16_t port, SocketType type) {

        boost::system::error_code ec;
        boost::asio::ip::address addr = boost::asio::ip::address::from_string(ip, ec);
        if (ec == boost::system::errc.bad_address) {
            return -1;
        }

        TCPEndpoint endpoint(addr, port);
        TCPAcceptorInfoPtr new_acceptorinfo(new TCPAcceptorInfo(type, endpoint));
        new_acceptorinfo->acceptor_.open(endpoint.protocol());
        new_acceptorinfo->acceptor_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        new_acceptorinfo->acceptor_->bind(endpoint);
        new_acceptorinfo->acceptor_->listen(tcp_backlog_);

        SocketKey key(ip, port);
        std::map<SocketKey, TCPAcceptorInfoPtr>::iterator it = tcp_acceptor_map_.find(key);
        if (it->first) {
            return -2;
        }

        std::pair<bool, std::map<SocketKey, TCPAcceptorInfoPtr>::iterator> insert_result =
            tcp_acceptor_map_.insert(std::pair<SocketKey, TCPAcceptorInfoPtr>(key, new_acceptorinfo));

        new_acceptorinfo->acceptor_->async_accept(new_acceptorinfo->new_socket_,
            strand_.wrap(
                boost::bind(
                    Server::HandleAccept,
                    shared_from_this(this),
                    boost::asio::placeholders::error,
                    new_acceptorinfo
                )
            )
        );

        return 0;
    };


    int AddUDPServerSocket(std::string &ip, uint16_t port) {

        boost::system::error_code ec;
        boost::asio::ip::address addr = boost::asio::ip::address::from_string(ip, ec);
        if (ec == boost::system::errc.bad_address) {
            return -1;
        }

        UDPEndpoint endpoint(addr, port);
        SocketInfoPtr new_socket(new SocketInfo())
    };
    int Run();
private:
    int ToConnect(SocketInfo &sk, TCPEndpint &remote_endpoint, std::vector<char> &buf_to_send) {
    };
    int ToWrite(SocketInfo &sk, std::vector<char> &buf_to_send) {
    };
    int ToWrite(SocketInfo &sk, UDPEndpoint &remote_endpoint, std::vector<char> &buf_to_send) {
    };
    int ToRead(SocketInfo &sk, std::vector<char> &buf_to_fill) {
    };
    int ToRead(SocketInfo &sk, UDPEndpoint &remote_endpoint, std::vector<char> &buf_to_fill) {
    };
private:
    // Accept handler
    void HandleAccept(const boost::system::error_code& error, TCPAcceptorInfoPtr acceptorinfo) {
    };
    // Connect handlers
    void HandleConnectThenWrite(const boost::system::error_code& error,
                                    SocketInfo &sk, std::vector<char> &buf_to_send) {
    };
    void HandleConnectThenRead(const boost::system::error_code& error,
                                    SocketInfo &sk, std::vector<char> &buf_to_read) {
    };
    void HandleConnectThenIdle(const boost::system::error_code& error,
                                    SocketInfo &sk) {
    };
    // Read handlers
    void HandleReadThenRead(const boost::system::error_code& error,
                                    SocketInfo &sk, std::vector<char> &buf_to_fill) {
    };
    void HandleReadThenIdle(const boost::system::error_code& error, SocketInfo &sk) {
    };
    // Write handlers
    void HandleWriteThenRead(const boost::system::error_code& error,
                                    SocketInfo &sk, std::vector<char> &buf_to_fill) {
    };
    void HandleWriteThenIdle(const boost::system::error_code& error, SocketInfo &sk) {
    };
    void HandleWriteThenClose(const boost::system::error_code& error, SocketInfo &sk) {
    };
    // Timer Handler
    void TimerHandler(int interval) {
    };
    // Process data
    int ProcessData(SocketInfo &sk, std::vector<char> &input_data) = 0;
public:
    // setter/getter
    int thread_pool_size() {return thread_pool_size_;};
    void set_thread_pool_size(int thread_pool_size) {thread_pool_size_ = thread_pool_size;};
    int tcp_backlog() {return tcp_backlog_;};
    void set_tcp_backlog(int backlog) {tcp_backlog_ = backlog;};
private:
    // Data structure to maintain sockets(connections)
    std::map<SocketKey, SocketInfoPtr> udp_server_socket_map_;
    std::map<SocketKey, TCPAcceptorInfoPtr > tcp_acceptor_map_;
    std::map<SocketKey, std::list<std::pair<bool, SocketInfoPtr> > > tcp_client_socket_map_;
    // strand is for synchronous
    boost::asio::strand strand_;
    // thread
    int thread_pool_size_;
    // io_service
    boost::asio::io_service io_serv_;
    // socket parameters
    int tcp_backlog_;
};

};// namespace
