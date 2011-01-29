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

#ifndef _ZXB_MEMBLOCK_H_
#define _ZXB_MEMBLOCK_H_

#include <pthread.h>

namespace ZXB {

class MemBlock {
public:
    void *start_;
    int used_;
    int len_;

private:
    MemBlock ();
    ~MemBlock();
    // Prohibits
    MemBlock (MemBlock &);
    MemBlock& operator= (MemBlock &);
};

class MemPool {
// For maintaining
public:
    int Get(int size, MemBlock *&mb);
    int Return(MemBlock *mb);
    int PrintStatus();
    int EnlargeMemPool(int size_to_add);
    static int CreateMemPool(int init_pool_size, int max_block_size, int block_size_step, MemPool *&mp);

private:
    MemPool();
    ~MemPool();
    pthread_mutex_t pool_lock_;
    // MemBlock size is 1024*n, so there are n-sized vector
    // to maintain the recycled MemBlocks
    std::vector<std::list<MemBlock*> > recycled_block_lists_;
    std::vector<std::vector<char> > mem_pool_;

    int unorganized_byte_num_;
    int free_item_pos_;
    size_t free_start_pos_;  // The memory not organized in MemBlock is at
                                    // 'mem_pool_[free_item_pos_].data() + free_start_pos_'

    // For statistics
    std::vector<uint64_t> block_get_num_;

    // Configurable parameters(can't be changed after initiated)
    int max_block_size_;
    int block_size_step_;
};
};
#endif
