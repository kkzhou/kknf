#ifndef _MESSAGE_INFO_HPP_
#define _MESSAGE_INFO_HPP_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>

namespace AANF {

class MessageInfo : public boost::enable_shared_from_this, private boost::noncopyable {
public:
    MessageInfo()
        :len_(0){};
    int len(){return len_;};
    uint16_t from_port() {return from_port_;};
    std::string& from_ip(){return from_ip_;};
    uint16_t to_port() {return to_port;};
    std::string& to_ip(){return to_ip;};
    boost::posix_time::ptime& arrive_time(){return arrive_time_;};
    boost::shared_ptr<std::vector<char> > data(){return data_;};

    void set_len(int len) {len_ = len;};
    void set_from_port(uint16_t from_port){ from_port_ = from_port;};
    void set_from_ip(std::string &from_ip){from_ip_ = from_ip;};
    void set_to_port(uint16_t to_port){ to_port_ = to_port;};
    void set_to_ip(std::string &to_ip){to_ip_ = to_ip;};
    void set_arrive_time(boost::posix_time::ptime &arrive_time){arrive_time_ = arrive_time;};
    void data(boost::shared_ptr<std::vector<char> > data){data_ = data;};
private:
    int len_;
    std::string from_ip_;
    uint16_t from_port_;
    std::string to_ip_;
    uint16_t to_port_;
    boost::posix_time::ptime arrive_time_;

    boost::shared_ptr<std::vector<char> > data_;
};
};
#endif
