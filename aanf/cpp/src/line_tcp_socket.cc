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


#define MAX_IOVEC_NUM 1000

namespace AANF {

#define NON_PACKETIZED_BUFFER_SIZE 1048576 // 1M

LineTcpSocket::LineTcpSocket() {

    MemPool::GetMemPool()->GetMemBlock(NON_PACKETIZED_BUFFER_SIZE, non_packetized_buf_);
}

LineTcpSocket::~LineTcpSocket() {

    MemPool::GetMemPool()->ReturnMemBlock(non_packetized_buf_);
}

// 实际完成对套接口读操作的函数。
// 返回值：
// 0: 还未读完一个包
// <0: 出现错误，应该关闭套接口或者重新连接
// 1: 读完一个包
int LineTcpSocket::ReadHandler() {

    int read_num = 0；

    read_num = read(fd_, non_packetized_buf_->start_ + non_packetized_buf_->used_,  len - non_packetized_buf_->used_);
    if (read_num <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0) {
            // 表明该套接口没有数据可读，应该来说不会达到这里，
            // 因为ReadHandler函数是在该套接口可读时才调用的。
            return 0;
        }
        // 否则，说明套接口出现错误
        return -2;
    }

    // 检查是否已经读入了'\n'
    char *start_pos = non_packetized_buf_->start_;
    char *pos1 = start_pos + non_packetized_buf_->used_;
    non_packetized_buf_->used_ += read_num; // 更新已使用长度
    char *pos2, *endpos;
    pos2 = endpos = non_packetized_buf_->used_ ;

    while (pos2 > pos1) {
        if (*pos2 == '\n') {
            break;
        }
    }

    if (pos2 == pos1) {
        // non_packetized_buf_里还没有一个完整的行
        return 0;
    }

    // 否则，在[start_pos, pos2]之间至少有一行，构造一个MemBlock，并交给调用者去组包packetize
    // 并且，把[pos2+1, endpos]之间的数据移动到non_packetized_buf的开头
    assert(recv_mb_ == 0);
    int ret = MemPool::GetMemPool()->GetMemBlock(pos2 - start_pos + 1, recv_mb_);
    if (ret < 0) {
    }

    memcpy(recv_mb_->start_, start_pos, pos2 - start_pos + 1);
    recv_mb_->used_ = pos2 - start_pos + 1;

    memmove(non_packetized_buf_->start_, pos2 + 1, endpos - pos2);
    non_packetized_buf_->used_ = endpos - pos2;
    return 1;
}

// 实际实现对套接口的写操作的函数。
// 返回值：
// 0: 未把队列中的所有数据写完
// 1: 已把队列中的数据写完，应该把该套接口从epoll中去除
// <0: 出现错误，应该关闭或者重连
int BinSocketOperator::WriteHandler() {

    list<MemBlock*>::iterator it = send_mb_list_.begin();
    list<MemBlock*>::iterator endit = send_mb_list_.end();

    struct iovec vec[MAX_IOVEC_NUM];
    int cnt = 0;
    for (; it != endit; it++) {
        vec[cnt].iov_base = *it->start_;
        vec[cnt].iov_len = *it->used_;
        cnt++;
        if (cnt == MAX_IOVEC_NUM - 1) {
            break;
        }
    }

    int write_num = writev(fd_, vec, cnt);
    if (write_num <= 0) {

        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0) {
            // 表明该套接口不可写，应该来说不会达到这里，
            return 0;
        }
        // 否则，说明套接口出现错误
        return -2;
    }

    int tmp_num = write_num;
    for (int i = 0; i <= cnt; i++) {
        if (tmp_num >= vec[i].iov_len) {
            // 这个io块已经发送出去了
            MemBlock *tmpmb = send_mb_list_.front();
            send_mb_list_.pop_front();
            MemPool::GetMemPool()->ReturnMemBlock(tmpmb);
            tmp_num -= vec[i].len;
        } else {
            // 这个块没有发送完
            font_mb_cur_pos_ = tmp_num;
            break;
        }
    }
    if (send_mb_list_.size() ==0 ) {
        return 1;
    }
    return 0;
}
}; // namespace AANF
