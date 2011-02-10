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

#include "zxb_bin_socket_operator.h"
#include "zxb_memblock.h"

namespace ZXB{

BinSocketOperator::BinSocketOperator() : socket_(0) {
}

BinSocketOperator::~BinSocketOperator() {
}

// 从套接口里读数据，如果已经成一个包（packet）则通过out参数返回
// return value:
// 0: 成功，但是还未读完一个数据包（packet）
// <0: 失败，需要调用ErrorHandler
// 1: 成功，且已经读完一个完整的数据包
int BinSocketOperator::ReadHandler(Packet *&in_pack) {

    int fd = socket()->fd_;
    // 如果recv_mb不存在，则分配一个，并且表明现在是从一个数据包的开头开始接收。
    if (socket()->recv_mb_ == 0) {
        if ((socket()->recv_mb_ = MemPool::GetMemPool()) == 0) {
            return -1;
        }
    }
    // 开始接收
    MemBlock *mb = socket()->recv_mb_;
    if (mb->used_ < sizeof(uint32_t)) {
        int read_num = read(fd, mb->start_ + mb->used_, sizeof(uint32_t) - mb->used_);
        if (read_num <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0) {
                // 表明该套接口没有数据可读，应该来说不会达到这里，
                // 因为ReadHandler函数是在该套接口可读时才调用的。
                return 0;
            }
            // 否则，说明套接口出现错误
            return -2;
        }

        if (read_num + mb->used_ < sizeof(uint32_t)) {
            // 表明还未把len域读出来，不必继续读了，等下一次可读时再继续
            return 0;
        }

        mb->used_ += read_num;
    }

    // 否则，表明len域已经读出，应该继续读一次
    uint32_t len = htonl(*reinterpret_cast<uint32_t*>(mb->start_));
    if (len > mb->len_) {
        // 如果len域的值比recv_mb的空间大，则需要重新获取一个MemBlock
        MemBlock *new_mb = 0;
        if (MemPool::GetMemPool()->Get(len, new_mb) < 0) {
            return -3;
        }
        memcpy(new_mb->start_, mb->start_, sizeof(uint32_t));
        new_mb->used_ += sizeof(uint32_t);
        MemPool::GetMemPool()->Return(mb);
        socket()->recv_mb_ = new_mb;
        mb = new_mb;
    }
    read_num = read(fd, mb->start_ + mb->used_,  len - sizeof(uint32_t));
    if (read_num <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == 0) {
            // 表明该套接口没有数据可读，应该来说不会达到这里，
            // 因为ReadHandler函数是在该套接口可读时才调用的。
            return 0;
        }
        // 否则，说明套接口出现错误
        return -2;
    }

    mb->used_ += read_num;
    if (read_num == len - sizeof(uint32_t)) {
        // 已经读完一个数据包了
        socket()->recv_mb_ = 0;
        struct timeval nowtime;
        gettimeofday(&nowtime);
        Packet *new_pkt = new Packet(nowtime, socket()->peer_ipstr_, socket()->peer_port_,
                                     socket()->my_ipstr_, socket()->my_port_,
                                     socket()->type_, socket()->data_format_, mb);
        in_pack = new_pkt;
        return 1;
    }
    return 0;
}

int BinSocketOperator::WriteHandler() {

    int fd = socket()->fd_;

    list<MemBlock*>::iterator it = socket()->send_mb_list_.begin();
    list<MemBlock*>::iterator endit = socket()->send_mb_list_.end();
#define MAX_IOVEC_NUM 1000
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

    int write_num = writev(fd, vec, cnt);
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
            MemBlock *tmpmb = socket()->send_mb_list_.front();
            socket()->send_mb_list_.pop_front();
            MemPool::GetMemPool()->Return(tmpmb);
            tmp_num -= vec[i].len;
        } else {
            // 这个块没有发送完
            socket()->font_mb_cur_pos_ = tmp_num;
            break;
        }
    }
    return 0;
}

};// namespace ZXB
