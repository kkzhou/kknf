#include <boost/asio/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <map>
#include <list>
#include <vector>
#include <iostream>


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
    T_TCP_UNKNOWN
};
// Simple information about a socket
class SocketInfo {
public:
    SocketInfo(SocketType type, boost::asio::io_service &io_serv)
        : type_(type),
        tcp_sk_(io_serv),
        in_use_(false),
        is_client_(false) {

        access_time_ = create_time_ =
            boost::posix_time::second_time::localtime();

    };
    SocketType type() {return type_;};
    bool is_client() {return is_client_;};
    void set_is_client(bool is_client) {is_client_ = is_client;};
    bool in_use() {return in_use_;};
    void Use() { BOOST_ASSERT(in_use_ == false); in_use_ = true;};
    TCPSocket& tcp_sk() {return tcp_sk_;};
    PTime& create_time() {return create_time;};
    PTime& access_time_() {return access_time_;};
    void Touch() {access_time_ = boost::posix_time::second_time::localtime();};
    std::vector<char>& recv_buf() {return recv_buf_;};
    void set_recv_buf(size_t byte_num) {
        std::vector<char> tmp;
        tmp.reserve(byte_num);
        recv_buf_.swap(tmp);
    };

private:
    SocketType type_;
    TCPSocket tcp_sk_;
    PTime create_time_;
    PTime access_time_;
    bool in_use_;
    bool is_client_;
    std::vector<char> recv_buf_;

};

// Information about an acceptor
class TCPAcceptorInfo {
public:
    TCPAcceptorInfo(SocketType type, boost::asio::io_service &io_serv)
        : type_(type),
        acceptor_(io_serv),
        sk_info_(new TCPSocket(type, io_serv)) {

    };
public:
    TCPAcceptor& acceptor() {return acceptor_;};
    SocketType type() {return type_;};
    TCPSocketPtr sk_info() {return sk_info_;};
    void Reset() {sk_info_.reset(new new TCPSocket(type, acceptor_.get_io_service());};
private:
    TCPAcceptor acceptor_;
    SocketType type_;
    SocketInfoPtr sk_info_;
};

typedef boost::asio::ip::tcp::socket TCPSocket;
typedef boost::shared_ptr<TCPSocket> TCPSocketPtr;
typedef boost::asio::ip::tcp::endpoint TCPEndpoint;
typedef boost::asio::ip::tcp::acceptor TCPAcceptor;
typedef boost::shared_ptr<TCPAcceptorInfo> TCPAcceptorInfoPtr;

typedef boost::shared_ptr<SocketInfo> SocketInfoPtr;

typedef boost::date_time::posix_time PTime;


class Server : public enable_shared_from_this<Server> {
public:
    Server()
        : strand_(io_serv_) {
    };
    int AddTCPListenSocket(std::string &ip, uint16_t port, SocketType type) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << endl;
        boost::system::error_code ec;
        boost::asio::ip::address addr = boost::asio::ip::address::from_string(ip, ec);
        if (ec) {
            std::cerr << "Error: " << ec.message() << endl;
            if (ec == boost::system::errc.bad_address) {
                std::cerr << "Address format error: " << ip << endl;
            }
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
            return -1;
        }

        TCPEndpoint endpoint(addr, port);
        TCPAcceptorInfoPtr new_acceptorinfo(new TCPAcceptorInfo(type, endpoint, io_serv_));
        new_acceptorinfo->acceptor().open(endpoint.protocol());
        new_acceptorinfo->acceptor().set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        new_acceptorinfo->acceptor().bind(endpoint);
        new_acceptorinfo->acceptor().listen(tcp_backlog_);

        SocketKey key(ip, port);
        std::map<SocketKey, TCPAcceptorInfoPtr>::iterator it = tcp_acceptor_map_.find(key);
        if (it->first) {
            std::cerr << "This acceptor already exists" << endl;
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
            return -2;
        }

        std::pair<bool, std::map<SocketKey, TCPAcceptorInfoPtr>::iterator> insert_result =
            tcp_acceptor_map_.insert(std::pair<SocketKey, TCPAcceptorInfoPtr>(key, new_acceptorinfo));

        new_acceptorinfo->acceptor().async_accept(new_acceptorinfo->sk_info()->tcp_sk(),
            strand_.wrap(
                boost::bind(
                    Server::HandleAccept,
                    shared_from_this(this),
                    boost::asio::placeholders::error,
                    new_acceptorinfo
                )
            )
        );
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
        return 0;
    };
    int Run();
private:
    int ToConnect(SocketInfoPtr skinfo, TCPEndpint &remote_endpoint, std::vector<char> &buf_to_send) {
    };
    int ToWrite(SocketInfoPtr skinfo, std::vector<char> &buf_to_send) {
    };
    int ToRead(SocketInfoPtr skinfo) {

        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&((skinfo->recv_buf()->at(0)), 4)),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &Server::HandleReadThenRead,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    };

    int DestroySocket(SocketInfoPtr skinfo) {
        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << endl;
        BOOST_ASSERT(skinfo->in_use());

        skinfo->tcp_sk().close();
        SocketKey key(skinfo->tcp_sk().remote_point().address().tostring(),
                      skinfo->tcp_sk().remote_point().port());
        if (skinfo->is_client()) {
            std::map<SocketKey, std::list<SocketInfoPtr> >::iterator it =
                tcp_client_socket_map_.find(key);

            if (it == tcp_client_socket_map_.end()) {
                std::cerr << "The SocketInfo to delete doesn't exist." << endl;
                std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
                return -1;
            }


            it->second.remove(skinfo);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
        return 0;
    };
private:
    // Accept handler
    void HandleAccept(const boost::system::error_code& error, TCPAcceptorInfoPtr acceptorinfo) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << endl;
        if (error) {
            std::cerr << "Accept error: " << ec.message() << endl;
            exit(1);
            return;
        }
        SocketInfoPtr sk_info = acceptorinfo->sk_info();
        acceptorinfo->Reset();
        sk_info->Use();

        SocketKey key(sk_info->tcp_sk().remote_point().address().tostring(),
                      sk_info->tcp_sk().remote_point().port());
        std::map<SocketKey, SocketInfoPtr>::iterator it
            = tcp_server_socket_map_.find(key);

        if (it->first) {
            std::cerr << "This server socket already exists(cant be true)" << endl;
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
            return -2;
        }

        std::pair<bool, std::map<SocketKey, SocketInfoPtr>::iterator> insert_result =
            tcp_server_socket_map_.insert(std::pair<SocketKey, SocketInfoPtr>(key, sk_info));

        boost::system::error_code read_ec;
        sk_info->set_recv_buf(init_recv_buf_size_);
        if (ToRead(sk_info) < 0) {
            std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
            Destroy(sk_info);
            return 0;
        }
        acceptorinfo->acceptor().async_accept(acceptorinfo->sk_info().tcp_sk(),
            strand_.wrap(
                boost::bind(
                    Server::HandleAccept,
                    shared_from_this(this),
                    boost::asio::placeholders::error,
                    acceptorinfo
                )
            )
        );
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
        return 0;

    };
    // Connect handlers
    void HandleConnectThenWrite(const boost::system::error_code& error, SocketInfoPtr skinfo,
                                    std::size_t byte_num, std::vector<char> &buf_to_send) {
    };
    void HandleConnectThenRead(const boost::system::error_code& error,
                               SocketInfoPtr skinfo, std::size_t byte_num) {
    };
    void HandleConnectThenIdle(const boost::system::error_code& error,
                               SocketInfoPtr skinfo, std::size_t byte_num) {
    };
    // Read handlers
    void HandleReadThenRead(const boost::system::error_code& error,
                            SocketInfoPtr skinfo, std::size_t byte_num) {

        if (error) {
            std::cerr << "Read error: " << error.message() << endl;
            exit(1);
            return;
        }
        int len = *(int*)(&((skinfo->recv_buf().at(0))));
        len = boost::asio::detail::socket_ops::network_to_host_long(len);
        if (len > max_recv_buf_size) {
            return;
        }

        if (len > skinfo->recv_buf().capacity()) {
            skinfo->set_recv_buf(len);
            *(int*)(&skinfo->recv_buf().at(0)) = boost::asio::detail::socket_ops::network_to_host_long(len);
        }
        // 开始读剩余的内容
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&((skinfo->recv_buf().at(4)), len - 4)),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &Server::HandleReadThenProcess,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));


    };
    void HandleReadThenProcess(const boost::system::error_code& error,
                               SocketInfoPtr skinfo, std::size_t byte_num) {
    };
    // Write handlers
    void HandleWriteThenRead(const boost::system::error_code& error,
                             SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << endl;
        boost::asio::async_read(
            skinfo->tcp_sk(),
            boost::asio::buffer(&((recv_buffer()->at(0)), 4)),
            boost::asio::transfer_all(),
            strand().wrap(
                boost::bind(
                    &Server::HandleReadThenRead,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));

        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
    };

    void HandleWriteThenIdle(const boost::system::error_code& error,
                             SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << endl;
        BOOST_ASSERT(skinfo->is_client());
        if (error) {
            std::cerr << "Write error: " << error.message() << endl;
            DestroySocket(skinfo);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
    };
    void HandleWriteThenClose(const boost::system::error_code& error,
                              SocketInfoPtr skinfo, std::size_t byte_num) {

        std::cerr << "Enter " << __FUNCTION__ << ":" << __LINE__ << endl;
        BOOST_ASSERT(skinfo->is_client());
        if (error) {
            DestroySocket(skinfo);
        }
        std::cerr << "Leave " << __FUNCTION__ << ":" << __LINE__ << endl;
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
    int max_recv_buf_size() {return max_recv_buf_size_;};
    void set_max_recv_buf_size(int max_recv_buf_size) {max_recv_buf_size_ = max_recv_buf_size;};
    int init_recv_buf_size(){ return init_recv_buf_size_;};
    void set_init_recv_buf_size(int init_recv_buf_size) {init_recv_buf_size_ = init_recv_buf_size;};
private:
    // Data structure to maintain sockets(connections)
    std::map<SocketKey, SocketInfoPtr> tcp_server_socket_map_;
    std::map<SocketKey, TCPAcceptorInfoPtr > tcp_acceptor_map_;
    std::map<SocketKey, std::list<SocketInfoPtr> > tcp_client_socket_map_;
    // strand is for synchronous
    boost::asio::strand strand_;
    // thread
    int thread_pool_size_;
    // io_service
    boost::asio::io_service io_serv_;

    // socket parameters
    int tcp_backlog_;
    int max_recv_buf_size_;
    int init_recv_buf_size_;
};

};// namespace
