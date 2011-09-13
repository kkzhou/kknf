#ifndef SOCKETADDR_H_
#define SOCKETADDR_H_
namespace NF
{

class SocketAddr {
public:
    std::string ip_;
    uint16_t port_;
public:
    bool operator< (const SocketAddr &o) const {
        if (ip_ < o.ip_) {
            return true;
        } else if (ip_ > o.ip_) {
            return false;
        } else {
            if (port_ < o.port_) {
                return true;
            } else {
                return false;
            }
        }
    };
};


class Packet {
public:
    SocketAddr from_;
    SocketAddr to_;
    std::vector<char> data_;
    struct timeval arrive_time_;
};

};
#endif
