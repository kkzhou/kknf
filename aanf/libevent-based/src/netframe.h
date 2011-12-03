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

#ifndef _NETFRAME_H_
#define _NETFRAME_H_

#include <sys/socket.h>
#include <string>
#include <event.h>

#include "socket.h"
#include "memblock.h"

namespace AANF {

class NetFrame;

// libevent的Callback函数的参数
// 套接口的回调函数所使用的参数
class CallBackArg {
public:
    NetFrame::FDType type_;
};

class SocketCallBackArg : public CallBackArg {
public:
    Socket *sk_;
    NetFrame *netframe_;
};


// libevent回调函数的原型
typedef (void)(*CallbackForLibEvent)(int, short, void*);

class NetFrame {
public:
    enum FDType {
        T_SOCKET = 1,
        T_SIGNAL,
        T_TIMER
    };

    enum SignalNo {
        SN_SEND_QUEUE_NOTIFY = 100,
        SN_USR_1 = 101,
        SN_USR_2 = 102,
        SN_USR_3 = 103,
        SN_USR_4 = 104,
        SN_USR_5 = 105,
        SN_USR_6 = 106,
        SN_USR_7 = 107,
        SN_USR_8 = 108,
        SN_USR_9 = 109,
        SN_USR_10 = 110
    };

public:
    static void SocketCallback(int fd, short events, void *arg);

public:
    NetFrame(int send_queue_num, int recv_queue_num);
    ~NetFrame();
    SocketPool* socket_pool();
    int set_max_recv_queue_size(int max_size);
    int set_max_send_queue_size(int max_size);

    // 这是信号处理函数。
    // 当worker线程要发送数据时，先把数据放到send_queue(s)里，然后发送信号，触发该函数。
    static void SendQueuesHandler(int signo, short events, void *arg);

    // 向系统中添加事件和处理函数
    // 添加一个套接口给libevent侦听
    int AddSocketToMonitor(Socket *sk);
    // 添加一个定时器给libevent
    int AddTimerToMonitor(CallbackForLibEvent cb, CallBackArg *cb_arg, int timeout_usec, int timer_id);
    // 添加一个信号给libevent
    int AddSignalToMonitor(CallbackForLibEvent cb, CallBackArg *cb_arg, int signo);
    // 进入消息循环
    int Run();// Loop

    // 主线程调用该函数把数据包放到接收队列，worker线程从中去取
    int PushMessageToRecvQueue(Message *in_pack);
    // worker线程调用该函数把数据包从接收线程里取出来
    int GetMessageFromRecvQueue(int which_queue, Message *&pack);
    // worker线程在处理数据包时，如果需要异步发送数据包，则调用该函数；
    // 该函数只是把数据包放到发送队列
    int AsyncSend(std::string &to_ipstr, uint16_t to_port,
                  std::string &my_ipstr, uint16_t my_port,
                  MemBlock *data, Socket::SocketType type,
                  Socket::DataFormat data_format, int which_queue);
private:

    // libevent
    struct event_base *ev_base_;

    // socket pool
    SocketPool *socket_pool_;

    // receive queues
    std::vector<std::list<Message*> > recv_queues_;
    std::vector<pthread_mutex_t*> recv_queue_locks_;// It's dangerous to copy a pthread_mutex_t so we use pointer
    std::vector<pthread_cond_t*> recv_queue_conds_;
    int max_recv_queue_size_;
    // send queues
    std::vector<std::list<Message*> > send_queues_;
    std::vector<pthread_mutex_t*> send_queue_locks_;// It's dangerous to copy a pthread_mutex_t so we use pointer
    int max_send_queue_size_;
    // Prohibits
    NetFrame(NetFrame&);
    NetFrame& operator=(NetFrame&);
};

}; // namespace AANF

#endif
