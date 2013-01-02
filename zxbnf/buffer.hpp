
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <deque>
#include <iostream>

namespace ZXBNF {

    class Buffer {
    public:
	Buffer(char *start, usigned int length) {
	    start_ = start;
	    length_ = length;
	    next_ = 0;
	};
	inline char* &start() { return start_; };
	inline unsigned int &length() { return length_; };
	inline Buffer* &next() { return next_; };
    private:
	char *start_;
	unsigned int length_;
	Buffer *next_;

	// forbid
	Buffer(Buffer&) {};
	Buffer& operator=(Buffer&) {};
    };

    // implement two memory pools(large and small), and the
    // blocks in these pools are in the same size
    class MemPool {
    public:
	MemPool(unsigned int large_block_size = 2*1024*1024, 
		unsigned int large_block_num = 100,
		unsigned int small_block_size = 4*1024, 
		unsigned int small_block_num = 1000) 
	    :idle_large_blocks_(0),
	     idle_small_blocks_(0) {

		large_block_size_ = large_block_size;
		large_block_num_ = large_block_num;
		small_block_size_ = small_block_size;
		small_block_num_ = small_block_num;

		large_pool_ = malloc(large_block_size_ * large_block_num_);
		small_pool_ = malloc(small_block_size_ * small_block_num_);

	
		for ( unsigned int i = 0; i < large_block_num_; i++) {
		    idle_large_blocks_.push_back(large_pool_ + i * large_block_size_);
		}
		for ( unsigned int i = 0; i < large_block_num_; i++) {
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
	};

	inline void Status() {
	
	};

    private:
	// large blocks
	char *large_pool_;

	std::deque<char*> idle_large_blocks_;
	unsigned int large_block_size_;
	unsigned int large_block_num_;

	// small blocks
	char *small_pool_;

	std::deque<char*> idle_small_blocks_;
	unsigned int small_block_size_;
	unsigned int small_block_num_;
    
	// forbid
	MemPool(MemPool &) {};
	MemPool& operator=(MemPool&) {};
    };
}; // namespace ZXBNF


#endif
