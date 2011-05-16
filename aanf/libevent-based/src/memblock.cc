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

#include "utils.h"
#include "memblock.h"

using namespace std;

namespace ZXB {

MemPool *MemPool::mem_pool_ = 0;

MemBlock::MemBlock ()
    :start_(0), curpos_(0),
     len_(0)
{
    ENTERING;
    LEAVING;
}

MemPool::MemPool() {

    ENTERING;
    pthread_mutex_init(&pool_lock_, NULL);
    unorganized_byte_num_ = 0;
    free_item_pos_ = ;
    free_start_pos_ = 0;
    max_block_size_ = 0;
    block_size_step_ = 0;
    LEAVING;
}

int MemPool::CreateMemPool (int init_pool_size, int max_block_size, int block_size_step, MemPool *&mp)
{
    ENTERING;
    if (mem_pool_) {
        mp = mem_pool_;
        SLOG(LogLevel::L_LOGICERR, "already created\n");
        LEAVING;
        return -1;
    }
    mem_pool_ = mp = new MemPool;
    // Do some initiates
    mp->max_block_size_ = max_block_size;
    mp->block_size_step_ = block_size_step;
    //
    int ret = mp->EnlargeMemPool (init_pool_size);
    LEAVING;
    return ret;
}

MemPool* MemPool::GetMemPool() {

    ENTERING;
    LEAVING;
    return mem_pool_;
}

int MemPool::EnlargeMemPool (int size_to_add)
{
    ENTERING;
    assert(unorganized_byte_num_ == 0);// 只有在为分派的内存大小为0时，才会调用本函数

    if (size_to_add <= 0) {
        SLOG(LogLevel::L_LOGICERR, "size_to_add invalid\n");
        LEAVING;
        return -1;
    }

    // 对要增加的大小进行调整，让他是基本块大小的整数倍
    int size_added = size_to_add / block_size_step_ * block_size_step_ + block_size_step_;
    if (size_added == size_to_add + block_size_step_) {
        size_added -= block_size_step_;
    }

    // 分配一整块内存
    vector<char> newbuf;
    newbuf.reserve(size_added);

    {
        Locker locker(&pool_lock_);
        if (unorganized_byte_num_ > 0) {
            // Other thread has done EnlargeMemPool
            SLOG(LogLevel::L_LOGICERR, "another thread has done EnlargeMemPool\n");
            LEAVING;
            return -2;
        }
        // 这里用一个小trick
        // 先放进去一个大小为0的vector，然后用swap来交换内存
        // 因为，如果直接把新分配的大整块内存push_back进去，会导致内存拷贝。
        mem_pool_.push_back(vector<char>(0));// Copy constructor is invoked, so use a trick
        size_t num = mem_pool_.size () - 1;
        swap (newbuf, mem_pool_[num]);// The trick is 'swap'
        block_get_num_.push_back(0);

        unorganized_byte_num_ += size_added;
    }

    LEAVING;
    return 0;
}


int MemPool::GetMemBlock(int size, MemBlock *&mb)
{
    ENTERING;
    if (size <= 0 || size > max_block_size) {
        SLOG(LogLevel::L_LOGICERR, "size invalid\n");
        LEAVING;
        return -1;
    }

    // 我们只能分配大小为block_size_step的整数倍的内存
    int real_size = size / block_size_step_ * block_size_step_ + block_size_step_;

    if (real_size == size + block_size_step_) {
        real_size -= block_size_step_;
    }

    // 先查看回收的内存块里有没有合适大小的
    int multiple  = real_size / block_size_step_;
    MemBlock *ret_mb = 0;

    Locker locker(&pool_lock_);

    block_get_num_[multiple ]++;

    if (recycled_block_lists_[multiple ].size() == 0) {
        // 如果回收列表里根本没有内存块，则找去找未分配的整块内存去要

        int left_size = mem_pool_[free_item_pos_].size() - free_start_pos_;
        if (left_size < real_size) {
            // 当前整块内存中，剩余的空间不够
            if (free_item_pos_ == mem_pool_.size() - 1) {
                // 当前整块内存就是最后一块，那需要向系统申请另一块整块内存
                // 即调用一下EnlargeMemPool
                return -2;
            }

            // 如果当前整块内存不是最后一块，但是剩余的空间也不够本次调用所请求的大小，
            // 那么，首先把剩余的空间分配出去，原则是：按被请求次数最多的那个size来分配
            // 然后以下一个整块内存作为当前整块内存。
            int tmp_max = 0;
            int alloc_for_which = 0;
            vector<int>::iterator endit = block_get_num_[multiple ].end();
            vector<int>::iterator it = block_get_num_[multiple ].begin();
            for (; it != endit; it++) {
                tmp_max = tmp_max < *it ? *it : tmp_max;
                alloc_for_which++;
            }
            // Allocate MemBlocks
            list<MemBlock*> new_list;
            MemBlock *new_block;
            while (left_size > alloc_for_which * block_size_step_) {

                new_block = new MemBlock;
                left_size -= alloc_for_which * block_size_step_;

                new_block->start_ = mem_pool_[free_item_pos_].front() + free_start_pos_;
                new_block->len_ = alloc_for_which * block_size_step_;
                new_block->curpos_ = 0;
                free_start_pos_ += new_block->len_;
                unorganized_byte_num_ -= new_block->len_;
                new_list.push_back(new_block);
            }
            recycled_block_lists_[multiple ].insert(new_list.begin(), new_list.end());

            if (left_size != 0) {
                assert(left_size % block_size_step_ == 0);
                new_block = new MemBlock;

                new_block->start_ = mem_pool_[free_item_pos_].front() + free_start_pos_;
                new_block->len_ = left_size;
                new_block->curpos_ = 0;
                free_start_pos_ += new_block->len_;
                unorganized_byte_num_ -= new_block->len_;

                assert(free_start_pos_ == mem_pool_[free_item_pos_].size());
                recycled_block_lists_[left_size / block_size_step_].push_back(new_block);
                left_size = 0;
            }
            // Then focus on the next vector in mem_pool
            free_item_pos_++;
            free_start_pos_ = 0;
            left_size = mem_pool_[free_item_pos_].size() - free_start_pos_;
        } //if (left_size < real_size)

        // 到这里，整块内存已经可用了（有足够空间满足本次请求）
        ret_mb = new MemBlock;
        ret_mb->start_ = mem_pool_[free_item_pos_].front() + free_start_pos_;
        ret_mb->len_ = real_size;
        ret_mb->curpos_ = 0;
        free_start_pos_ += ret_mb->len_;
        unorganized_byte_num_ -= ret_mb->len_;
    } else {
        // 在回收的列表里可以找到
        ret_mb = recycled_block_lists_[multiple ].front();
        recycled_block_lists_[multiple ].pop_front();
    }
    mb = reb_mb;
    LEAVING;
    return 0;
}

int MemPool::ReturnMemBlock(MemBlock *mb)
{
    ENTERING;
    Locker locker(&pool_lock_);
    int multiple  = mb->len_ / block_size_step_;
    mb->curpos_ = 0;
    recycled_block_lists_[multiple].push_back(mb);
    LEAVING;
    return 0;
}
};
