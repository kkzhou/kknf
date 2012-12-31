
#ifndef __ZXB_CONNECTION_H__

#include "zxb_buffer.hpp"
#include <list>

namespace ZXBNF {

class Connection {
public:
    Connection();
private:
    std::list<Buffer*> send_buffer_;
    std::list<Buffer*> recv_buffer_;
};

}// namespace ZXBNF

#define __ZXB_CONNECTION_H__
#endif
