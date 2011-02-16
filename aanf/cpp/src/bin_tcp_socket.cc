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

#include "bin_tcp_socket.h"


#define MAX_IOVEC_NUM 1000

namespace AANF {


// 实际完成对套接口读操作的函数。
// 返回值：
// 0: 还未读完一个包
// <0: 出现错误，应该关闭套接口或者重新连接
// 1: 读完一个包
int BinTcpSocket::ReadHandler() {

    int read_num = 0；
    // 开始接收
    if (len_field_read_ < sizeof(uint32_t)) {
        // 长度域还没有读完
        char *tmpptr = reinterpret_cast<char*>(&len_field_);
        read_num = read(fd_, tmpptr + len_field_read_, sizeof(uint32_t) - len_field_read_);
        if (read_num <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0) {
                // 表明该套接口没有数据可读，应该来说不会达到这里，
                // 因为ReadHandler函数是在该套接口可读时才调用的。
                return 0;
            }
            // 否则，说明套接口出现错误
            return -2;
        }
        len_field_read_ += read_num;
        if (len_field_read_ < sizeof(uint32_t)) {
            // 表明还未把len域读出来，不必继续读了，等下一次可读时再继续
            return 0;
        } else {
            assert(len_field_read_ == sizeof(uint32_t));
            assert(recv_mb_ == 0);  // 在长度域还未读完之前，这个指针是空的
            int real_len = ntohl(len_field_);
            recv_mb_ = MemPool::GetMemPool()->GetMemBlock(real_len);
            memcpy(recv_mb_->start_, &len_field_, sizeof(uint32_t));
            recv_mb_->used_ += sizeof(uint32_t);
        }
    }

    // 长度域已经读完，开始读数据
    uint32_t len = htonl(*reinterpret_cast<uint32_t*>(recv_mb_->start_));
    assert(len <= recv_mb_->len_);

    read_num = read(fd_, recv_mb_->start_ + recv_mb_->used_,  len - recv_mb_->used_);
    if (read_num <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0) {
            // 表明该套接口没有数据可读，应该来说不会达到这里，
            // 因为ReadHandler函数是在该套接口可读时才调用的。
            return 0;
        }
        // 否则，说明套接口出现错误
        return -2;
    }

    recv_mb_->used_ += read_num;
    if (recv_mb_->used_ == len) {
        // 已经读完一个数据包了
        return 1;
    }
    return 0;
}

}; // namespace AANF
