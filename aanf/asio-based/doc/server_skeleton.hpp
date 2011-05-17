#include <boost/asio/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <map>
#include <list>
#include <vector>

class SocketKey {
public:
    string ip_;
    uint16_t port_;
};

class ServerSkeleton {
public:
    typedef boost::asio::ip::udp::socket UDPSocket;
    typedef boost::shared_ptr<UDPSocket> UDPSocketPtr;
    typedef boost::asio::ip::udp::endpoint UDPEndpoint;
    typedef boost::shared_ptr<UDPEndpoint> UDPEndpointPtr;

    typedef boost::asio::ip::tcp::socket TCPSocket;
    typedef boost::shared_ptr<TCPSocket> TCPSocketPtr;
    typedef boost::asio::ip::tcp::endpoint TCPEndpoint;
    typedef boost::shared_ptr<TCPEndpoint> TCPEndpointPtr;
public:
    int AddTCPListenSocket();
    int AddUDPListenSocket();
    int Run();
private:
    int TCPToConnect(TCPSocket &sk, TCPEndpint &remote_endpoint, std::vector<char> &buf_to_send) {
    };
    int TCPToWrite(TCPSocket &sk, std::vector<char> &buf_to_send) {
    };
    int TCPToRead(TCPSocket &sk, std::vector<char> &buf_to_fill) {
    };
    int UDPToWrite(UDPSocket &sk, UDPEndpoint &remote_endpoint, std::vector<char> &buf_to_send) {
    };
    int UDPToRead(UDPSocket &sk, UDPEndpoint &remote_endpoint, std::vector<char> &buf_to_fill) {
    };
private:
    // Connect handlers
    void TCPHandleConnectThenWrite(const boost::system::error_code& error, 
                                    TCPSocket &sk, std::vector<char> &buf_to_send) {
    };
    void TCPHandleConnectThenRead(const boost::system::error_code& error, 
                                    TCPSocket &sk, std::vector<char> &buf_to_read) {
    };
    void TCPHandleConnectThenIdle(const boost::system::error_code& error, 
                                    TCPSocket &sk) {
    };
    // Read handlers
    void TCPHandleReadThenRead(const boost::system::error_code& error, 
                                    TCPSocket &sk, std::vector<char> &buf_to_fill) {
    };
    void TCPHandleReadThenIdle(const boost::system::error_code& error, TCPSocket &sk) {
    };
    // Write handlers
    void TCPHandleWriteThenRead(const boost::system::error_code& error, 
                                    TCPSocket &sk, std::vector<char> &buf_to_fill) {
    };
    void TCPHandleWriteThenIdle(const boost::system::error_code& error, TCPSocket &sk) {
    };
    void TCPHandleWriteThenClose(const boost::system::error_code& error, TCPSocket &sk) {
    };
private:
    std::map<SocketKey, UDPSocketPtr> udp_listen_socket_map_;
    std::map<SocketKey, TCPSocketPtr> tcp_listen_socket_map_;
    std::map<SocketKey, std::list<std::pair<bool, TCPSocketPtr> > > tcp_client_socket_map_;
};
