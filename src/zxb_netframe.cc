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

#include "zxb_netframe.h"

namespace ZXB {

NetFrame* NetFrame::CreateNetFrame() {

    NetFrame *nf = 0;
    if (nf)
        return nf;

    nf = new NetFrame;
    return nf;
}
NetFrame::NetFrame(){
}
NetFrame::~NetFrame(){
}
int NetFrame::AddSocketToMonitor(Socket *sk) {

    if (!sk) {
        return -1;
    }
    // The Socket is ready for monitor, that is, we can put it
    // in epoll/select or sth. else.
    // Actually we should check the invalidation of 'sk'
    struct event ev;
    short events = EV_PERSIST;
    if (sk->events_concern_ & Socket::EV_READ) {
        events |= EV_READ;
    }
    if (sk->events_concern_ & Socket::EV_WRITE) {
        events |= EV_WRITE;
    }

    SocketCallBackArg *arg = new SocketCallBackArg;
    arg->type_ = CallBackArg::T_SOCKET;
    arg->sk_ = sk;
    arg->nf_ = this;
    event_set(&ev, sk->fd_, events, NetFrame::SocketCallback, arg);

    if (-1 == event_base_set(ev_base_, &ev)) {
        return -2;
    }

    if (-1 == event_add(&ev, NULL)) {
        return -2;
    }

    return 0;
}

int NetFrame::AddSignalToMonitor(CallBackForLibEvent cb,
                                 CallBackArg *cb_arg, int signo) {

    if (signo < 0) {
        return -1;
    }
    struct event ev;
    short events = EV_PERSIST;

    CallBackArg *arg = new CallBackArg;
    arg->type_ = CallBackArg::T_SIGNAL;
    arg->nf_ = this;
    event_set(&ev, signo, events, cb, arg);

    if (-1 == event_base_set(ev_base_, &ev)) {
        return -2;
    }

    if (-1 == event_add(&ev, NULL)) {
        return -2;
    }

    return 0;
}

int NetFrame::AddTimerToMonitor(CallBackForLibEvent cb,
                                CallBackArg *cb_arg,
                                int timeout_usec, int timer_id) {

    if (signo < 0) {
        return -1;
    }
    struct event ev;
    short events = EV_PERSIST;

    TimerCallBackArg *arg = new TimerCallBackArg;
    arg->type_ = CallBackArg::T_TIMER;
    arg->timer_id_ = time_id;
    arg->nf_ = this;
    gettimeofday(&arg->trig_time_);
    arg->trig_time_.tv_usec += timeout_usec % 1000000;
    arg->trig_time_.tv_sec += timeout_usec / 1000000 + arg->trig_time_.tv_usec / 1000000;
    arg->trig_time_.tv_usec %= 1000000;
    event_set(&ev, signo, events, cb, arg);

    if (-1 == event_base_set(ev_base_, &ev)) {
        return -2;
    }

    if (-1 == event_add(&ev, NULL)) {
        return -2;
    }

    return 0;
}

int NetFrame::Run() {
    event_base_dispatch(ev_base_);
    return 0;
}

void NetFrame::SocketCallback(int fd, short events, void *arg) {

    CallBackArg cb_arg1 = reinterpret_cast<CallBackArg*>(arg);
    if (cb_arg1->type_ != CallBackArg::T_SOCKET) {
        // It's not a socket event
        return;
    }
    SocketCallBackArg cb_arg = reinterpret_cast<SocketCallBackArg*>(arg);
    Socket *sk = cb_arg->sk_;
    // Handle the socket

    short events_concern = 0;
    if (sk->event_concern_ | Socket::EV_READ) {
        events_concern |= EV_READ;
    }
    if (sk->event_concern_ | Socket::EV_WRITE) {
        events_concern |= EV_WRITE;
    }
    // read
    int read_result = 0;
    int process_result = 0;
    if ((sk->status_ == Socket::S_ESTABLISHED) && (events_concern | EV_READ) && (events | EV_READ))) {
        Packet *pack_read = NULL;
        read_result = sk->op_->ReadHandler(pack_read);
    }

    if (read_result < 0) {
        // If error happens in reading the only thing we should do is closing it
        sk->op_->ErrorHandler(SocketOperator::C_CLOSE);
        return;
    } else if (read_result == 1) {
        // A complete packet has been read
        // process it
        process_result = cb_arg->nf_->ProcessPacket(pack_read);
        if (process_result == -1) {
            // receive queue(s) are full
            return;
        }
    }
    // write
    int write_result = 0;
    if ((sk->status_ == Socket::S_ESTABLISHED) && (events_concern | EV_WRITE) && (events | EV_WRITE)) {
        // data transmiting
        write_result = sk->op_->WriteHandler();
    }

    if (read_result < 0) {
        if (sk->type_ == Socket::T_TCP_SERVER)
            // If error happens in writing and I am a server, just close it
            sk->op_->ErrorHandler(SocketOperator::C_CLOSE);
            return;
        } else {
            sk->op_->ErrorHandler(SocketOperator::C_RECONNECT);
            return;
        }
    }

    // connect
    write_result = 0;
    if ((sk->status_ == Socket::S_CONNECTING) && (events_concern | EV_WRITE) && (events | EV_WRITE)) {
        // non-blocking connect
        int error = 0;
        if (SocketOperator::GetSocketError(sk->fd_, error) < 0 || error != 0) {
            sk->op_->ErrorHandler(SocketOperator::C_CLOSE);
            return;
        }
        // try to write
        write_result = sk->op_->WriteHandler();
        if (write_result < 0) {
            sk->op_->ErrorHandler(SocketOperator::C_CLOSE);
            return;
        }
    }

    // accept
    int accept_result = 0;
    if ((sk->status_ == Socket::S_LISTEN) && (events_concern | EV_READ) && (events | EV_READ)) {

        accept_result = sk->op_->AcceptHandler();
        if (accept_result < 0) {
            // What can we do?
            return;
        }
    }
}

int NetFrame::SetupRecvQueues(int queue_num) {

    if (queue_num <= 0) {
        return -1;
    }

    for (int i=0; i < queue_num; i++) {

        recv_queues_.push_back(list<Packet*>(0));

        pthread_mutex_t *new_mutex = new pthread_mutex_t;
        *new_mutex = PTHREAD_MUTEX_INITIALIZER;
        recv_queue_locks_.push_back(new_mutex);

        pthread_cond_t *new_cond = new pthread_cond_t;
        *new_cond = PTHREAD_COND_INITIALIZER;
        recv_queue_conds_.push_back(new_cond);
    }
    return 0;
}

int NetFrame::ProcessPacket(Packet *in_pack) {

    if (!in_pack) {
        return -1;
    }

    int tmp_min = numerical<int>::max();
    int tmp_which = 0;
    for (int i = 0; i < recv_queues_.size(); i++) {
        if (recv_queues_[i].size() < tmp_min) {
            tmp_min = recv_queues_[i].size();
            tmp_which = i;
        }
    }

    {
        ConditionVariable cv(recv_queue_locks_[tmp_which], recv_queue_conds_[tmp_which]);
        recv_queues_[tmp_which].push_back(in_pack);
        if (recv_queues_[tmp_which].size() == 1) {
            cv.Signal();
        }
    }
    return 0;
}

int NetFrame::GetPacketFromeQueue(int which_queue, Packet *&pack) {

    if (which_queue < 0 || which_queue >= recv_queues_.size()) {
        return -1;
    }

    {
        ConditionVariable cv(recv_queue_locks_[tmp_which], recv_queue_conds_[tmp_which]);
        while (recv_queues_[which_queue].size() == 0) {
            cv.Wait();
        }
        pack = recv_queues_[which_queue].front();
        recv_queues_[which_queue].pop_front();
    }
    return 0;
}

int NetFrame::AsyncSend(std::string &to_ipstr, uint16_t to_port, std::string &my_ipstr, uint16_t my_port,
                        MemBlock *data, enum SocketType type, int &socket_fd_used) {

    int send_result = 0;
    Socket *sk_used = NULL;
    send_result = SocketOperator::AsyncSend(to_ipstr, to_port, my_ipstr, my_port, data, type, sk_used);
    if (send_result < 0) {
        return -2;
    }
    socket_fd_used = sk_used->fd_;
    return 0;

}

};
