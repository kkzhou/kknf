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

#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

#include <deque>
#include <iostream>

namespace NF {

    class Block {
    public:
	Block(char *s, int l)
	    : start_(s),
	      length_(l) {
	};
	char* Data() {
	    return start_;
	};
	int Size() {
	    return length_;
	};
    private:
	char *start_;
	int length_;
    };

    // implement two memory pools(large and small), and the
    // blocks in these pools are in the same size
    class MemPool {
    public:
	MemPool(int large_block_size = 2*1024*1024, 
		int large_block_num = 100,
		int small_block_size = 4*1024, 
		int small_block_num = 1000) 
	    :idle_large_blocks_(0),
	     idle_small_blocks_(0) {

	    large_block_size_ = large_block_size;
	    large_block_num_ = large_block_num;
	    small_block_size_ = small_block_size;
	    small_block_num_ = small_block_num;

	    large_pool_ = malloc(large_block_size_ * large_block_num_);
	    small_pool_ = malloc(small_block_size_ * small_block_num_);

	
	    for (int i = 0; i < large_block_num_; i++) {
		idle_large_blocks_.push_back(large_pool_ + i * large_block_size_);
	    }

	    for (int i = 0; i < large_block_num_; i++) {
		idle_large_blocks_.push_back(large_pool_ + i * large_block_size_);
	    }
	};

	inline void Destroy() {
	    delete[] large_pool_;
	    delete[] small_pool_;
	};

	inline Block* GetSmallBlock() {
	    
	    if (idle_small_blocks_.empty()) {
		return 0;
	    }

	    Block *b = new Buffer(idle_small_blocks_.front(), small_block_size_);
	    idle_small_blocks_.pop_front();
	    return b;
	};

	inline Block* GetLargeBlock() {
	    
	    if (idle_large_blocks_.empty()) {
		return 0;
	    }

	    Block *b = new Block(idle_large_blocks_.front(), large_block_size_);
	    idle_large_blocks_.pop_front();
	    return b;
	};

	inline void PutBlock(Block *b) {
	
	    assert(b);
	
	    if (b->length() == large_block_size_) {
		assert((b->start() - large_pool_) % large_block_size_ == 0);
		idle_large_blocks_.push_front(buf->start());
	    } else if (b->length() == small_block_size_) {
		assert((b->start() - small_pool_) % small_block_size_ == 0);
		idle_small_blocks_.push_front(buf->start());
	    } else {
		assert(false);
	    }
	    delete b;
	};

	inline void Status() {
	
	};
	
	inline int small_block_size() { return small_block_num_; };
	inline int large_block_size() { return large_block_size_; };

    private:
	// large blocks
	char *large_pool_;

	std::deque<char*> idle_large_blocks_;
	int large_block_size_;
	int large_block_num_;

	// small blocks
	char *small_pool_;

	std::deque<char*> idle_small_blocks_;
	int small_block_size_;
	int small_block_num_;
    };
}; // namespace NF


#endif
