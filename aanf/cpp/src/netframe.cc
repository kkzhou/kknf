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

#include "netframe.h"

namespace AANF {


NetFrame::NetFrame(int send_queue_num, int recv_queue_num) {

    if (recv_queue_num <= 0 || send_queue_num) {
        return -1;
    }

    for (int i = 0; i < recv_queue_num; i++) {

        recv_queues_.push_back(list<Packet*>(0));

        pthread_mutex_t *new_mutex = new pthread_mutex_t;
        pthread_mutex_init(new_mutex, NULL);
        recv_queue_locks_.push_back(new_mutex);

        pthread_cond_t *new_cond = new pthread_cond_t;
        pthread_cond_init(new_cond, NULL);
        recv_queue_conds_.push_back(new_cond);
    }

    for (int i = 0; i < send_queue_num; i++) {

        send_queues_.push_back(list<MemBlock*>(0));

        pthread_mutex_t *new_mutex = new pthread_mutex_t;
        pthread_mutex_init(new_mutex, NULL);
        send_queue_locks_.push_back(new_mutex);
    }

    socket_pool_ = new SocketPool;
}

NetFrame::~NetFrame(){
    // 系统终止，需要释放所有资源
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
    arg->type_ = NetFrame::T_SOCKET;
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

    cb_arg->type_ = NetFrame::T_SIGNAL;
    event_set(&ev, signo, events, cb, cb_arg);

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

    cb_arg->type_ = CallBackArg::T_TIMER;
    cb_arg->timer_id_ = time_id;
    gettimeofday(&cb_arg->trig_time_);
    cb_arg->trig_time_.tv_usec += timeout_usec % 1000000;
    cb_arg->trig_time_.tv_sec += timeout_usec / 1000000 + cb_arg->trig_time_.tv_usec / 1000000;
    cb_arg->trig_time_.tv_usec %= 1000000;
    event_set(&ev, signo, events, cb, cb_arg);

    if (-1 == event_base_set(ev_base_, &ev)) {
        return -2;
    }

    if (-1 == event_add(&ev, NULL)) {
        return -2;
    }

    return 0;
}

int NetFrame::Run() {
    // 添加几个预置的handler，例如处理发送队列通知信号的handler
    CallBackArg *cb_arg = new CallBackArg;
    AddSignalToMonitor(SendQueuesHandler, cb_arg, SN_SEND_QUEUE_NOTIFY);

    event_base_dispatch(ev_base_);
    return 0;
}

void NetFrame::SocketCallback(int fd, short events, void *arg) {

    SocketCallBackArg cb_arg = reinterpret_cast<SocketCallBackArg*>(arg);
    if (cb_arg->type_ != NetFrame::T_SOCKET) {
        // It's not a socket event
        return;
    }

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
        read_result = sk->ReadHandler();
    }

    if (read_result < 0) {
        // If error happens in reading the only thing we should do is closing it
        socket_pool_->DestroySocket(sk);
        return;
    } else if (read_result == 1) {
        // A complete packet has been read
        // process it
        struct timeval nowtime;
        gettimeofday(&nowtime);
        Packet *pkt_read = new Packet(nowtime, sk->peer_ipstr_, sk->peer_port_,
                                     sk->my_ipstr_, sk->my_port_,
                                     sk->type_, sk->data_format_, sk->recv_mb_);
        sk->recv_mb_ = 0;
        process_result = cb_arg->nf_->PushPacketToRecvQueue(pkt_read);
        if (process_result == -1) {
            // receive queue(s) is full
            return;
        }
    } else {
    }

    // write
    int write_result = 0;
    if ((sk->status_ == Socket::S_ESTABLISHED) && (events_concern | EV_WRITE) && (events | EV_WRITE)) {
        // data transmiting
        write_result = sk->WriteHandler();
    }

    if (write_result < 0) {
        if (sk->type_ == Socket::T_TCP_SERVER) {
            // If error happens in writing and I am a server, just close it
            socket_pool_->DestroySocket(sk);
            return;
        } else {
            // I am client, so reconnect
            if (sk->Reconnect() == 0) {
                AddSocketToMonitor(sk);
            } else {
                socket_pool_->DestroySocket(sk);
            }

            return;
        }
    } else if (write_result == 1) {
        // 缓存的数据已经写完，把套接口的写事件从epoll中删除
        sk->event_concern ^= (~Socket::EV_WRITE);
        AddSocketToMonitor(sk);
    }

    // connect
    write_result = 0;
    if ((sk->status_ == Socket::S_CONNECTING) && (events_concern | EV_WRITE) && (events | EV_WRITE)) {
        // non-blocking connect
        // check error
        int error = 0;
        if (sk->GetSocketError(error) < 0 || error != 0) {
            socket_pool_->DestroySocket(sk);
            return;
        }
        // try to write
        write_result = sk->WriteHandler();
        if (write_result < 0) {
            if (sk->Reconnect() == 0 ) {
                AddSocketToMonitor(sk);
            } else {
                socket_pool_->DestroySocket(sk);
            }
            return;
        }
    }

    // accept
    if ((sk->status_ == Socket::S_LISTEN) && (events_concern | EV_READ) && (events | EV_READ)) {

        int newfd = accept(fd, NULL, NULL);
        Socket *newsk = socket_pool_->CreateServerSocket(newfd, sk->type_, sk->data_format_);
        AddSocketToMonitor(newsk);

    }
    return;
}

int NetFrame::PushPacketToRecvQueue(Packet *in_pack) {

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

int NetFrame::GetPacketFromRecvQueue(int which_queue, Packet *&pack) {

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

int NetFrame::AsyncSend(std::string &to_ipstr, uint16_t to_port,
                        std::string &my_ipstr, uint16_t my_port,
                        MemBlock *data, Socket::SocketType type,
                        Socket::DataFormat data_format, int which_queue) {

    if (to_ipstr.empty() || to_port == 0 ||
        (type != Socket::T_TCP_CLIENT && type != Socket::T_UDP_CLIENT)) {

        return -1;
    }

    {
        Locker Locker(send_queue_locks_[which_queue]);
        struct timeval nowtime;
        gettimeofday(&nowtime);
        Packet *pkt = new Packet(nowtime, to_ipstr, to_port,
                                 my_ipstr, my_port, type, data_format, data);
        send_queues_[which_queue].push_back(pkt);
    }
    return 0;
}

void NetFrame::SendQueuesHandler(int signo, short events, void *arg) {

    if (signo != SN_SEND_QUEUES_NOTIFY) {
        return;
    }
    CallBackArg *cb_arg = reinterpret_cast<CallBackArg*>(arg);

    // 把发送队列的所有MemBlock交给Socket对象
    for (int i = 0; i < send_queues_.size(); i++) {

        Locker locker(send_queue_locks_[i]);
        while (send_queues_[i].size() > 0) {

            Packet *pkt = send_queues_[i].front();
            send_queues_[i].pop_front();

            Socket *sk = SocketPool::GetSocketPool()->FindSocket(pkt->my_ipstr_, pkt->my_port_, pkt->sk_type_);
            if (sk) {
                // The socket alreay exists, just send to it
                sk->PushDataToSend(pkt->data_);
                continue;
            }
            // 如果不存在这个套接口，则要先创建，并且添加到libevent的侦听事件中。
            // Create a new one which should act as CLINET
            SocketOperator *new_op = op_factory_->CreateSocketOperator(pkt->sk_type_, pkt->data_format_);
            sk = SocketPool::GetSocketPool()->CreateSocket(new_op);
            sk->my_ipstr_ = pkt->my_ipstr_;
            sk->my_port_ = pkt->my_port_;
            sk->type_ = pkt->sk_type_;
            sk->event_concern_ = Socket::EV_READ | Socket::EV_WRITE | Socket::EV_ERROR;
            // 把数据放到该socket的发送缓存里，所有权就交给这个socket了，
            // 销毁也由它负责。
            sk->PushDataToSend(pkt->data_);
            // prepare socket
            if (sk->fd_ = socket(PF_INET, SOCK_STREAM, 0) < 0) {
                perror("socket() error: ");
                SocketPool::GetSocketPool()->DestroySocket(sk);
                delete pkt;
                continue;
            }

            // Set nonblocking
            int val = fcntl(sk->fd_, F_GETFL, 0);
            if (val == -1) {
                perror("Get socket flags error: ");
                SocketPool::GetSocketPool()->DestroySocket(sk);
                delete pkt;
                continue;
            }

            if (fcntl(sk->fd_, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
                perror("Set socket flags error: ");
                SocketPool::GetSocketPool()->DestroySocket(sk);
                delete pkt;
                continue;
            }

            // prepare address
            struct sockaddr_in to_addr;
            to_addr.sin_family = AF_INET;
            to_addr.sin_port = htons(to_port);
            if (inet_aton(to_ipstr.c_str(), &to_addr.sin_addr) == 0) {
                SocketPool::GetSocketPool()->DestroySocket(sk);
                delete pkt;
                continue;
            }
            // connect
            if (connect(sk->fd_, (struct sockaddr*)&to_addr, sizeof(struct sockaddr)) == -1) {
                if (errno != EINPROGRESS) {
                    perror("Connect error: ");
                    SocketPool::GetSocketPool()->DestroySocket(sk);
                    delete pkt;
                    continue;
                }
            }

            sk->status_ = Socket::S_CONNECTING;
            // 添加到一个libevent的事件
            if (AddSocketToMonitor(sk) < 0) {
                SocketPool::GetSocketPool()->DestroySocket(sk);
                delete pkt;
                continue;
            }
            // 成功创建新的套接口，并把数据交给它了。
        }// while
    } // for

}

}; // namespace AANF
