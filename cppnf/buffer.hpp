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

namespace ZXBNF {

    class Buffer {
    public:
	Buffer(char *start,  int length) {
	    start_ = start;
	    length_ = length;
	    head_ = 0;
	    tail_ = 0;
	};

	inline char* start() { return start_; };
	inline int length() { return length_; };
	inline int &head() { return head_; };
	inline int &tail() { return tail_;};
	inline Buffer* &next() { return next_; };
	inline Buffer* &prev() } return prev_; };

    private:
	char *start_;
	int head_;
	int tail_;
	int length_;
	Buffer *next_;
	Buffer *prev_;
    private:
	// forbid
	Buffer(Buffer&) {};
	Buffer& operator=(Buffer&) {};
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

	inline Buffer* GetSmallBlock() {
	    
	    if (idle_small_blocks_.empty()) {
		return 0;
	    }

	    Buffer *buf = new Buffer(idle_small_blocks_.front(), small_block_size_);
	    idle_small_blocks_.pop_front();
	    return buf;
	};

	inline Buffer* GetLargeBlock() {
	    
	    if (idle_large_blocks_.empty()) {
		return 0;
	    }

	    Buffer *buf = new Buffer(idle_large_blocks_.front(), large_block_size_);
	    idle_large_blocks_.pop_front();
	    return buf;
	};

	inline void PutBlock(Buffer *buf) {
	
	    assert(buf);
	
	    if (buf->length() == large_block_size_) {
		assert((buf->start() - large_pool_) % large_block_size_ == 0);
		idle_large_blocks_.push_front(buf->start());
	    } else if (buf->length() == small_block_size_) {
		assert((buf->start() - small_pool_) % small_block_size_ == 0);
		idle_small_blocks_.push_front(buf->start());
	    } else {
		assert(false);
	    }
	    delete buf;
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
    
	// forbid
	MemPool(MemPool &) {};
	MemPool& operator=(MemPool&) {};
    };
}; // namespace ZXBNF


#endif
