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

#include "zxb_memblock.h"

using namespace std;

namespace ZXB {

// Define static variables
pthread_mutex_t MemBlock::all_lock_ = PTHREAD_MUTEX_INITIALIZER;
vector<list<MemBlock*> > MemBlock::recycled_block_lists_;
vector<vector<char> > MemBlock::mem_pool_;
vector<int> MemBlock::block_get_num_;

int MemBlock::free_item_pos_ = 0;
size_t MemBlock::free_start_pos_ = 0;

int MemBlock::max_block_size_ = 0;
int MemBlock::block_size_step_ = 0;

MemBlock::MemBlock ()
    :start_(0), curpos_(0),
     len_(0)
{
}

int MemBlock::CreateMemPool (int init_size, int max_block_size, int block_size_step)
{
    // Do some initiates
    max_block_size_ = max_block_size;
    block_size_step_ = block_size_step;
    //
    int ret = EnlargeMemPool (init_size);
    return ret;
}

int MemBlock::EnlargeMemPool (int size_to_add)
{
    if (size_to_add <= 0) {
        return -1;
    }

    int size_added = size_to_add / block_size_step_ * block_size_step_ + block_size_step_;
    if (size_added == size_to_add + block_size_step_) {
        size_added -= block_size_step_;
    }

    vector<char> newbuf;
    newbuf.reserv (size_added);

    {
        Locker locker(&all_lock_);

        mem_pool_.push_back(vector<char>(0));// Copy constructor is invoked, so use a trick
        size_t num = mem_pool_.size () - 1;
        swap (newbuf, mem_pool_[num]);// The trick is 'swap'
        block_get_num_.push_back(0);
    }

    return 0;
}

// return:
// -1: parameter invalid
// -2: not enough space left
int MemBlock::Get (int size)
{
    if (size <= 0 || size > max_block_size) {
        return -1;
    }

    int real_size = size / block_size_step_ * block_size_step_ + block_size_step_;

    if (real_size == size + block_size_step_) {
        real_size -= block_size_step_;
    }
    // First check the recycled lists
    int multiple  = real_size / block_size_step_;
    MemBlock *ret_mb = 0;

    Locker locker(&all_lock_);

    block_get_num_[multiple ]++;

    if (recycled_block_lists_[multiple ].size() == 0) {

        int left_size = mem_pool_[free_item_pos_].size() - free_start_pos_;
        if (left_size < real_size) {
            // There is not enough space left in this item of the mem_pool
            if (free_item_pos_ == mem_pool_.size() - 1) {
                // Not enough free space in all mem_pool
                // Enlarge first
                return -2;
            }

            // we allocate this little framgment memory to the most-used-size block
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

                assert(free_start_pos_ == mem_pool_[free_item_pos_].size());
                recycled_block_lists_[left_size / block_size_step_].push_back(new_block);
                left_size = 0;
            }
            // Then focus on the next vector in mem_pool
            free_item_pos_++;
            free_start_pos_ = 0;
            left_size = mem_pool_[free_item_pos_].size() - free_start_pos_;
        } //if (left_size < real_size)

        // Now memory is ready, we just get one in MemBlock
        ret_mb = new MemBlock;
        ret_mb->start_ = mem_pool_[free_item_pos_].front() + free_start_pos_;
        ret_mb->len_ = real_size;
        ret_mb->curpos_ = 0;
        free_start_pos_ += ret_mb->len_;
    } else {
        // We have some recycled blocks left
        ret_mb = recycled_block_lists_[multiple ].front();
        recycled_block_lists_[multiple ].pop_front();
    }

    return reb_mb;
}

int MemBlock::Return()
{
    Locker locker(&all_lock_);
    int multiple  = len_ / block_size_step_;
    curpos_ = 0;
    recycled_block_lists_[multiple].push_back(this);
}
};
