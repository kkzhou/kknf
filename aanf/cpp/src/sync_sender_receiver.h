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
#ifndef _SYNC_SENDER_RECEIVER_H_
#define _SYNC_SENDER_RECEIVER_H_

#include <string>


namespace AANF {
class MemBlock;
// 同步发送接收器。
// 用于主动发送，并阻塞等待接收的场景；不用于先收后发的场景。
// 这是基类抽象类。
class SyncSR {
public:
    SyncSR();
    int CloseSR();
    int SetOpterationTimeout(int usec);
    // 初始化并连接，对于TCP来说，完成连接过程，对于udp来说则设定对端地址
    // （在recv的时候只收这个地址的数据，在send时，不指定对端地址时，默认发往这个地址）
    int ConnectToServer(std::string server_ip, uint16_t server_port, bool is_tcp);

    // 发送数据，并阻塞等待数据返回；由该函数确定何为一个数据包（即Pakcetize）
    // 可以指定超时的微秒数，如果指定为0则不超时
    virtual int SyncSendRecv(MemBlock *to_send, MemBlock *&to_recv) = 0;
private:
    int fd_;
    std::string server_ipstr_;
    uint16_t server_port_;
    bool is_tcp_;
};

// 处理二进制打包方式，即LV方式
class BinSyncSR : public SyncSR {
public:
    virtual int SyncSendRecv(MemBlock *to_send, MemBlock *&to_recv);
};

// 处理基于行的打包方式
class LineSyncSR : public SyncSR {
public:
    virtual int SyncSendRecv(MemBlock *to_send, MemBlock *&to_recv);
};

}; // namespace AANF
#endif
