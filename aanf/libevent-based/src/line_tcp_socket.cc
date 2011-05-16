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

#include "line_tcp_socket.h"

namespace AANF {

#define NON_PACKETIZED_BUFFER_SIZE 1048576 // 1M

LineTcpSocket::LineTcpSocket() {

    ENTERING;
    int ret = MemPool::GetMemPool()->GetMemBlock(NON_PACKETIZED_BUFFER_SIZE, non_packetized_buf_);
    if (ret < 0) {
        SLOG(LogLevel::L_FATAL, "can't allocate memory\n");
    }
    LEAVING;
}

LineTcpSocket::~LineTcpSocket() {

    ENTERING;
    int ret = MemPool::GetMemPool()->ReturnMemBlock(non_packetized_buf_);
    if (ret < 0) {
        SLOG(LogLevel::L_FATAL, "can't return the memblock\n");
    }
    LEAVING;
}


int LineTcpSocket::ReadHandler() {

    ENTERING;
    int read_num = 0；

    read_num = read(fd_, non_packetized_buf_->start_ + non_packetized_buf_->used_,
                    len - non_packetized_buf_->used_);

    if (read_num <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0) {
            // 表明该套接口没有数据可读，应该来说不会达到这里，
            // 因为ReadHandler函数是在该套接口可读时才调用的。
            SLOG(LogLevel::L_SYSERR, "no data to read: %s\n", strerror(errno));
            LEAVING;
            return 0;
        }
        // 否则，说明套接口出现错误
        SLOG(LogLevel::L_SYSERR, "read error: %s\n", strerror(errno));
        LEAVING;
        return -2;
    }

    // 检查是否已经读入了'\n'
    char *start_pos = non_packetized_buf_->start_;          // 记录缓存的开始处
    char *pos1 = start_pos + non_packetized_buf_->used_;    // 记录上次已经检查过缓存的末尾（因为这次读入后还未更新used_的值）
    non_packetized_buf_->used_ += read_num;                 // 更新已使用长度
    char *pos2, *endpos;
    pos2 = endpos = start_pos + non_packetized_buf_->used_ ;// 记录缓存的末尾

    while (pos2 > pos1) {
        if (*pos2 == '\n') {
            break;
        }
        pos2--;
    }

    if (pos2 == pos1) {
        // non_packetized_buf_里还没有一个完整的行
        SLOG(LogLevel::L_DEBUG, "not read a complete line\n");
        LEAVING;
        return 0;
    }

    // 否则，在[start_pos, pos2]之间至少有一行，构造一个MemBlock，并交给调用者去组包packetize
    // 并且，把[pos2+1, endpos]之间的数据移动到non_packetized_buf的开头
    assert(recv_mb_ == 0);
    int ret = MemPool::GetMemPool()->GetMemBlock(pos2 - start_pos + 1, recv_mb_);
    if (ret < 0) {
        SLOG(LogLevel::L_FATAL, "can't allocate a memblock\n");
    }

    memcpy(recv_mb_->start_, start_pos, pos2 - start_pos + 1);
    recv_mb_->used_ = pos2 - start_pos + 1;

    memmove(non_packetized_buf_->start_, pos2 + 1, endpos - pos2);
    non_packetized_buf_->used_ = endpos - pos2;
    LEAVING;
    return 1;
}

}; // namespace AANF
