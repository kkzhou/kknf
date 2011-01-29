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
class CallBackArg {
    enum EventType {
        T_SOCKET = 1,
        T_SIGNAL,
        T_TIMER
    };
public:
    int type_;
    NetFrame *nf_;
};

class SocketCallBackArg : public CallBackArg {
public:
    Socket *sk_;
};

class TimerCallBackArg : public CallBackArg {
public:
    int timer_id_;
    struct timeval trig_time_;
};

typedef (void)(*CallbackForLibEvent)(int, short, void*);

class NetFrame {
public:
    static void SocketCallback(int fd, short events, void *arg);

public:
    NetFrame *CreateNetFrame();
    // Manipulators
    int SetupRecvQueues(int queue_num);

    // Network I/O interfaces
    int AddSocketToMonitor(Socket *sk);
    int AddTimerToMonitor(CallbackForLibEvent cb, CallBackArg *cb_arg, int timeout_usec, int timer_id);
    int AddSignalToMonitor(CallbackForLibEvent cb, CallBackArg *cb_arg, int signo);
    int Run ();// Loop

    // Packet proccessing interface
    int ProcessPacket(Packet *in_pack);
    int GetPacketFromeQueue(int which_queue, Packet *&pack);// It's thread-safe
    int AsyncSend(std::string &to_ipstr, uint16_t to_port, std::string &my_ipstr, uint16_t my_port,
                              MemBlock *data, enum SocketType type, int &seq);// It's thread-safe


private:
    NetFrame();
    ~NetFrame();
    // libevent
    struct event_base *ev_base_;
    // receive queues
    std::vector<std::list<Packet*> > recv_queues_;
    std::vector<pthread_mutex_t*> recv_queue_locks_;// It's dangeraout to copy a pthread_mutex_t so we use pointer
    std::vector<pthread_cond_t*> recv_queue_conds_;

    // Prohibits
    NetFrame(NetFrame&);
    NetFrame& operator=(NetFrame&);
};

};

#endif
