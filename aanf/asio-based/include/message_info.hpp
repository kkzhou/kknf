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


#ifndef _MESSAGE_INFO_HPP_
#define _MESSAGE_INFO_HPP_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time.hpp>

#include <vector>

namespace AANF {

class MessageInfo : public boost::enable_shared_from_this, private boost::noncopyable {

public:
    MessageInfo(){};
    uint16_t from_port() {return from_port_;};
    std::string& from_ip(){return from_ip_;};
    uint16_t to_port() {return to_port;};
    std::string& to_ip(){return to_ip;};
    boost::posix_time::ptime& arrive_time(){return arrive_time_;};
    boost::shared_ptr<std::vector<char> > data(){return data_;};

    void set_from_port(uint16_t from_port){ from_port_ = from_port;};
    void set_from_ip(std::string &from_ip){from_ip_ = from_ip;};
    void set_to_port(uint16_t to_port){ to_port_ = to_port;};
    void set_to_ip(std::string &to_ip){to_ip_ = to_ip;};
    void set_arrive_time(boost::posix_time::ptime &arrive_time){arrive_time_ = arrive_time;};
    void set_data(boost::shared_ptr<std::vector<char> > data){data_ = data;};

private:
    std::string from_ip_;
    uint16_t from_port_;
    std::string to_ip_;
    uint16_t to_port_;
    boost::posix_time::ptime arrive_time_;

    boost::shared_ptr<std::vector<char> > data_;
};
};
#endif
