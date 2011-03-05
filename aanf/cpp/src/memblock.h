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

#ifndef _MEMBLOCK_H_
#define _MEMBLOCK_H_

#include <pthread.h>
#include <string>
#include <vector>
#include <list>

namespace AANF {

// MemBlock和MemPool两个类实现了一个简单的内存池。
// MemPool是管理者，MemBlock是容器。
// 其基本思想是：把整块内存（一块或多块）分成以1024的倍数（可调）为单位的MemBlock进行管理，
// 这个过程是在客户的实际请求的过程中实现的。
// 当客户释放MemBlock时，MemPool把它放到列表中（相同大小的放到一个列表）。
// 当再有客户请求相近大小的内存时，从列表中取，如果列表中没有，则从整块内存中创建新的MemBlock。
class MemBlock {
friend class MemPool;
public:
    void *start_;   // 该内存块的起始地址
    int used_;      // 该内存块已使用字节数
    int len_;       // 该内存块的总大小

private:
    MemBlock ();
    ~MemBlock();
    // 禁止对象拷贝
    MemBlock (MemBlock &);
    MemBlock& operator= (MemBlock &);
};

class MemPool {
public:
    // 获取一个MemBlock
    // 返回值：
    // 0: 成功
    // -1: 参数错误
    // -2: 内存不够，请先EnlargeMemPool
    int GetMemBlock(int size, MemBlock *&mb);
    // 归还一个MemBlock，不是被释放，而是被回收
    // 返回值：
    // 0: 成功
    // -1: 参数错误
    int ReturnMemBlock(MemBlock *mb);
    //　打印当前MemPool的状态，例如共有多少个MemBlock，用于调试。
    // 返回值：
    // 0: 成功
    // -1: 参数错误
    int PrintStatus();
    // 扩张整个内存池大小
    // 返回值：
    // 0: 成功
    // -1: 参数错误
    int EnlargeMemPool(int size_to_add);
    // 创建一个内存池，包装构造函数
    // 返回值：
    // 0: 成功
    // -1: 已经存在了，不能再次创建
    static int CreateMemPool(int init_pool_size, int max_block_size, int block_size_step, MemPool *&mp);
    // 获取当前的内存池的实例，没有把CreateMemPool函数的功能集合到这个函数，因为创建需要很多参数。
    // 返回值：
    // 内存池实例指针
    static MemPool* GetMemPool();

private:
    // 禁止外界直接构造内存池
    MemPool();
    ~MemPool();

    pthread_mutex_t pool_lock_; // 能锁住整个内存池所有操作的锁

    std::vector<std::list<MemBlock*> > recycled_block_lists_;// 回收回来的MemBlock，按大小用list维护
    std::vector<std::vector<char>* > mem_pool_;  // 大块内存，不够时再分配

    int unorganized_byte_num_;  // 为被组织成MemBlock的内存大小

    // 总体说来，未被组织成MemBlock的内存起始地址是；'mem_pool_[free_item_pos_].data() + free_start_pos_'
    int free_item_pos_;     // 当前，为被组织成MemBlock的内存的起始地址，在哪个大块内存
    size_t free_start_pos_;  // 当前，为被组织成MemBlock的内存的起始地址，在该打开内存块内的偏移


    // 统计量
    std::vector<uint64_t> block_get_num_; // 每种大小的内存块分别被请求的次数。

    // 可配置参数，决定了MemBlock的大小规则
    int max_block_size_;    // MemBlock的最大字节数
    int block_size_step_;   // MemBlock的递增步长，例如1024字节，则MemBlock的大小必是1024的倍数，且小于
                            // max_block_size_
    static MemPool *inst_;
};
}; // namespace AANF
#endif
