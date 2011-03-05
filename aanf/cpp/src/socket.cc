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
    ENTERING;
    fd_ = -1;
    type_ = T_TCP_CLIENT;
    status_ = S_NOTREADY;
    data_format_ = DF_BIN;

    my_port_ = 0;
    peer_port_ = 0;

    want_to_read_ = true;
    want_to_write_ = false;

    gettimeofday(&last_use_);
    retry_times_ = 0;
    pthread_mutex_init(&send_mb_list_lock_, NULL);
    font_mb_cur_pos_ = 0;
    LEAVING;
}
// 析构函数
// 需要释放资源，包括接受缓存、发送缓存列表
Socket::~Socket() {

    ENTERING;
    close(fd_);

    // Return the recv buf MemBlock
    recv_mb_->Return();
    recv_mb_ = 0;
    std::list<MemBlock*>::iterator it = send_mb_list_.begin();
    std::list<MemBlock*>::iterator endit = send_mb_list_.end();

    for (; it != endit; it++) {
        *it->Return();
    }
    LEAVING;
}

int Socket::PushDataToSend(MemBlock *mb)
{
    ENTERING;
    send_mb_list_.push_back(mb);
    gettimeofday(&last_use_);
    LEAVING;
    return 0;
}

int Socket::GetSocketError(int &error)
{
    ENTERING;
    error = 0;
    socklen_t optlen = sizeof(error);
    if (getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &optlen) < 0) {
        SLOG(LogLevel::L_SYSERR, "getsocketopt error\n");
        LEAVING;
        return -2;
    }
    LEAVING;
    return 0;
}

};// namespace AANF
#endif
