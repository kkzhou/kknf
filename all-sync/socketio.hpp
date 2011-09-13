#ifndef  __SOCKETIF_HPP__
#define  __SOCKETIF_HPP__

#include "socket.hpp"
#include "socketaddr.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace NF{

class SocketIO
{
public:
	//connection before TCPSend for client
	Socket* MakeConnection(std::string &ip, uint16_t port)
	{

		ENTERING;
		SLOG(2, "Make connection to <%s : %u>\n", ip.c_str(), port);
		int fd = -1;
		// prepare socket
		if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
				SLOG(2, "socket() error %s\n", strerror(errno));
				LEAVING;
				return 0;
		}

		SLOG(2, "socket() made a fd = %d\n", fd);
		struct sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);
		if (inet_aton(ip.c_str(), &server_addr.sin_addr) == 0) {
				SLOG(2, "inet_aton() error\n");
				LEAVING;
				return 0;
		}
		socklen_t addr_len = sizeof(struct sockaddr_in);

		// connect
		if (connect(fd, (struct sockaddr*)&server_addr, addr_len) == -1) {
				SLOG(2, "connect() error %s\n", strerror(errno));
				LEAVING;
				return 0;
		}

		// Get my_ip of this socket
		struct sockaddr_in myaddr;
		socklen_t myaddr_len = sizeof(struct sockaddr_in);
		if (getsockname(fd, (struct sockaddr*)&myaddr, &myaddr_len) == -1) {
				SLOG(2, "getsockname() error %s\n", strerror(errno));
				LEAVING;
				return 0;
		}

		std::string tmpip = inet_ntoa(myaddr.sin_addr);
		Socket *ret_sk = new Socket(fd);
		ret_sk->set_my_ipstr(tmpip);
		ret_sk->set_my_ip(myaddr.sin_addr);
		ret_sk->set_my_port(ntohs(myaddr.sin_port));
		ret_sk->set_peer_ip(server_addr.sin_addr);
		ret_sk->set_peer_ipstr(ip);
		ret_sk->set_peer_port(port);
		ret_sk->set_id(0xFFFFFFFF);
		ret_sk->set_type(5);
		SLOG(2, "Made a socket <%s : %u> -> <%s : %u>\n",
						ret_sk->peer_ipstr().c_str(),
						ret_sk->peer_port(),
						ret_sk->my_ipstr().c_str(),
						ret_sk->my_port());
		LEAVING;
		return ret_sk;
    };


    virtual int TCPRecv(Socket *sk, std::vector<char> &buf_to_fill) = 0;

    int TCPSend(Socket *sk, char *buf_to_send,
					uint32_t buf_to_send_size) {
        ENTERING;
        if (buf_to_send == 0) {
            SLOG(2, "Parameter error\n");
            LEAVING;
            return -1;
        }

        // send
        uint32_t size_left = buf_to_send_size;
        char *cur = buf_to_send;
        SLOG(2, "Begin to send %u bytes\n", buf_to_send_size);
        while (size_left) {

            int ret = send(sk->sk(), cur, size_left, 0);
            if (ret == -1) {
                SLOG(2, "send() error %s\n", strerror(errno));
                if (errno != EWOULDBLOCK || errno != EAGAIN) {
                    LEAVING;
                    return -2;
                }
            } else {
                size_left -= ret;
                cur += ret;
                SLOG(2, "%u bytes left\n", size_left);
            }
        }
        SLOG(2, "Send OK\n");
        LEAVING;
        return 0;
    };

    int UDPRecv(int sk, std::vector<char> &buf_to_fill, std::string &from_ip, uint16_t &from_port) {

        ENTERING;
        struct sockaddr_in from_addr;
        socklen_t addr_len;

        memset(&from_addr, sizeof(struct sockaddr_in), 0);
        addr_len = sizeof(struct sockaddr_in);

        buf_to_fill.resize(65535);
        // udp_socket_ is BLOCKING fd
        int ret = recvfrom(sk, &buf_to_fill[0], buf_to_fill.size(), 0,
                         (struct sockaddr*)(&from_addr), &addr_len);

        if (ret <= 0) {
            SLOG(2, "recvfrom() error: %s\n", strerror(errno));
            LEAVING;
            return -1;
        }

        from_ip = inet_ntoa(from_addr.sin_addr);
        from_port = ntohs(from_addr.sin_port);
        buf_to_fill.resize(ret);
        SLOG(2, "Recved %zd bytes from <%s : %u>\n", buf_to_fill.size(), from_ip.c_str(), from_port);
        LEAVING;
        return 0;
    };

    int UDPSend(int sk, std::string &to_ip, uint16_t to_port, char *buf_to_send, int buf_len) {

        ENTERING;
        struct sockaddr_in to_addr;
        socklen_t addr_len;

        SLOG(2, "Send to <%s : %u>\n", to_ip.c_str(), to_port);
        memset(&to_addr, sizeof(struct sockaddr_in), 0);
        to_addr.sin_family = AF_INET;
        to_addr.sin_port = htons(to_port);
        if (inet_aton(to_ip.c_str(), &to_addr.sin_addr) == 0) {
            SLOG(2, "inet_aton() error\n");
            LEAVING;
            return -1;
        }

        addr_len = sizeof(struct sockaddr_in);
        // udp_socket_ is BLOCKING fd
        int ret = sendto(sk, buf_to_send, buf_len, 0,
                         (struct sockaddr*)(&to_addr), addr_len);

        if (ret <= 0) {
            SLOG(2, "Send error: %s\n", strerror(errno));
            LEAVING;
            return -1;
        }
        LEAVING;
        return 0;
    };

	inline int MakeUDPSocket() {

        ENTERING;
        int fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (fd < 0) {
            SLOG(2, "Make udp socket error: %s\n", strerror(errno));
            LEAVING;
            return -1;
        }

        LEAVING;
        return fd;
    };
public:
	int max_udp_pkt_size() const
	{
		return max_udp_pkt_size_;
	}
	int max_tcp_pkt_size() const
	{
		return max_tcp_pkt_size_;
	}
private:
	const static int max_udp_pkt_size_ = 2048;
	const static int max_tcp_pkt_size_ = 2 * 1024 * 1024;
};
};//namespace
#endif
