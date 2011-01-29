 /*
    Copyright (C) <2011>  <ZHOU Xiaobo>

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

#ifndef _ZXB_MEMBLOCK_H_
#define _ZXB_MEMBLOCK_H_

#include <pthread.h>

namespace ZXB {

class MemBlock {

public:
    MemBlock ();
    // Allocator and Deallocator
    static int Get (int size, MemBlock *mb);    // Must be thread-safe
    int Return ();    // Must be thread-safe

public:
    void *start_;
    int curpos_;
    int len_;

// For maintaining
public:
    static int PrintStatus ();// Must be thread-safe
    static int EnlargeMemPool (int size_to_add);// Must be thread-safe
    static int CreateMemPool (int init_size, int max_block_size, int block_size_step);// Must be thread-safe

private:
    static pthread_mutex_t all_lock_;

    // MemBlock size is 128*n, so there are n-sized vector
    // to maintain the recycled MemBlocks

    static std::vector<std::list<MemBlock*> > recycled_block_lists_;

    static std::vector<std::vector<char> > mem_pool_;
    static int free_item_pos_;
    static size_t free_start_pos_;  // The memory not organized in MemBlock is at
                                    // 'mem_pool_[free_item_pos_].data() + free_start_pos_'

    // For statistics
    static std::vector<uint64_t> block_get_num_;

    // Configurable parameters(can't be changed after initiated)
    static int max_block_size_;
    static int block_size_step_;
private:
    // Prohibits
    ~MemBlock();
    MemBlock (MemBlock &);
    MemBlock& operator= (MemBlock &);
};

};
#endif
