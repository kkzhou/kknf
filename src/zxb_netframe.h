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

#ifndef _ZXB_NETFRAME_H_
#define _ZXB_NETFRAME_H_

#include <sys/socket.h>
#include <string>
#include <event.h>

#include "zxb_socket.h"
#include "zxb_socket_operator.h"
#include "zxb_memblock.h"

namespace ZXB {

class NetFrame;

// libevent的Callback函数的参数基类
class CallBackArg {
public:
    int type_;
    NetFrame *nf_;
};

// 套接口的回调函数所使用的参数
class SocketCallBackArg : public CallBackArg {
public:
    Socket *sk_;
};

// 定时器回调函数所使用的参数
class TimerCallBackArg : public CallBackArg {
public:
    int timer_id_;
    struct timeval trig_time_;
};

// 发送队列通知信号的回调函数所使用的参数
class SendQueuesNotifySignalCallBackArg : public CallBackArg {
public:
    int which_queue_;
};

// libevent回调函数的原型
typedef (void)(*CallbackForLibEvent)(int, short, void*);

class NetFrame {
public:
    enum EventType {
        T_SOCKET = 1,
        T_SIGNAL,
        T_TIMER
    };
    enum SignalNo {
        SN_SEND_QUEUE_NOTIFY = 101,
        SN_RELOAD_CONFIG = 102
    };
public:
    static void SocketCallback(int fd, short events, void *arg);
    int AcceptHandler();

public:
    NetFrame *CreateNetFrame();
    // Manipulators
    int SetupRecvQueues(int queue_num);
    int SetupSendQueues(int queue_num);
    void SetSocketOperatorFactory(SocketOperatorFactory *op_factory);
    // setter/getter
    MemPool* mempool();
    void set_mempool(MemPool *mp);

    // 这个信号的handler是用于业务线程通知libevent，有数据要发送。
    static void SendQueuesNoitfyHandler(int signo, short events, void *arg);

    // 向系统中添加事件和处理函数
    int AddSocketToMonitor(Socket *sk);// 添加一个套接口
    int AddTimerToMonitor(CallbackForLibEvent cb, CallBackArg *cb_arg, int timeout_usec, int timer_id);// 添加一个定时器
    int AddSignalToMonitor(CallbackForLibEvent cb, CallBackArg *cb_arg, int signo);// 添加一个信号
    int Run ();// Loop

    // Packet proccessing interface
    int ProcessPacket(Packet *in_pack);
    int GetPacketFromRecvQueue(int which_queue, Packet *&pack);
    // Socket manipulateors
    int PrepareListenSocket(std::string &my_ipstr, uint16_t my_port, Socket::SocketType type,
                            Socket::DataFormat data_format);

    int AsyncSend(std::string &to_ipstr, uint16_t to_port,
                  std::string &my_ipstr, uint16_t my_port,
                  MemBlock *data, Socket::SocketType type,
                  Socket::DataFormat data_format, int which_queue);
private:
    NetFrame();
    ~NetFrame();
    // libevent
    struct event_base *ev_base_;
    // receive queues
    std::vector<std::list<Packet*> > recv_queues_;
    std::vector<pthread_mutex_t*> recv_queue_locks_;// It's dangeraout to copy a pthread_mutex_t so we use pointer
    std::vector<pthread_cond_t*> recv_queue_conds_;
    // send queues
    std::vector<std::list<Packet*> > send_queues_;
    std::vector<pthread_mutex_t*> send_queue_locks_;// It's dangeraout to copy a pthread_mutex_t so we use pointer

    // MemPool
    MemPool *mempool_;
    // SocketPoll
    SocketPool *socket_pool_;
    // SocketOperator factory
    SocketOperatorFactory *op_factory_;

    // Prohibits
    NetFrame(NetFrame&);
    NetFrame& operator=(NetFrame&);
};

};

#endif
