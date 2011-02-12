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

#include "socket.h"
#include "memblock.h"
#include "utils.h"

namespace AANF {

// 构造函数
Socket::Socket() {
    fd_ = -1;
    type_ = T_TCP_CLIENT;
    status_ = S_NOTREADY;
    data_format_ = DF_BIN;

    my_port_ = 0;
    peer_port_ = 0;

    event_concern_ = EV_READ | EV_WRITE | EV_ERROR | EV_PERSIST;

    gettimeofday(&last_use_);
    retry_times_ = 0;
    pthread_mutex_init(&send_mb_list_lock_, NULL);
    font_mb_cur_pos_ = 0;
}
// 析构函数
// 需要释放资源，包括接受缓存、发送缓存列表
Socket::~Socket() {

    close(fd_);

    // Return the recv buf MemBlock
    recv_mb_->Return();
    recv_mb_ = 0;
    std::list<MemBlock*>::iterator it = send_mb_list_.begin();
    std::list<MemBlock*>::iterator endit = send_mb_list_.end();

    for (; it != endit; it++) {
        *it->Return();
    }
}

int Socket::PushDataToSend(MemBlock *mb)
{
    send_mb_list_.push_back(mb);
    gettimeofday(&last_use_);
    return 0;
}

int Socket::GetSocketError(int &error)
{
    error = 0;
    socklen_t optlen = sizeof(error);
    if (getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &optlen) < 0)
        return -2;

    return 0;
}

uint16_t Socket::event_concern() {
    return event_concern_;
}

void Socket::set_event_concern(uint16_t events) {
    event_concern_ = events;
}

uint16_t Socket::add_event_to_concern(Event ev) {
    event_concern_ |= ev;
    return event_concern_;
}

uint16_t Socket::del_event_to_concern(Event ev) {
    event_concern_ &= (~ev);
    return event_concern_;
}

};// namespace AANF
#endif
