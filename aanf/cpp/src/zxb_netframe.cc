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
MemPool* NetFrame::mempool() {
    return mempool_;
}
void NetFrame::set_mempool(MemPool *mp) {
    mempool_ = mp;
}
void NetFrame::SetSocketOperatorFactory(SocketOperatorFactory *op_factory) {

    op_factory_ = op_factory;
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
    // 添加几个预置的handler，例如处理发送队列通知信号的handler
    CallBackArg *cb_arg = new CallBackArg;
    cb_arg->nf_ = this;
    cb_arg->type_ = EV_SIGNAL;
    AddSignalToMonitor(SendQueuesNoitfyHandler, cb_arg, SN_SEND_QUEUE_NOTIFY);

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
        SocketOperator::ErrorHandler(sk, SocketOperator::C_CLOSE);
        SocketPool::GetSocketPool()->DestroySocket(sk);
        return;
    } else if (read_result == 1) {
        // A complete packet has been read
        // process it
        process_result = cb_arg->nf_->ProcessPacket(pack_read);
        if (process_result == -1) {
            // receive queue(s) are full
            return;
        }
    } else {
    }
    // write
    int write_result = 0;
    if ((sk->status_ == Socket::S_ESTABLISHED) && (events_concern | EV_WRITE) && (events | EV_WRITE)) {
        // data transmiting
        write_result = sk->op_->WriteHandler();
    }

    if (write_result < 0) {
        if (sk->type_ == Socket::T_TCP_SERVER)
            // If error happens in writing and I am a server, just close it
            SocketOperator::ErrorHandler(sk, SocketOperator::C_CLOSE);
            SocketPool::GetSocketPool()->DestroySocket(sk);
            return;
        } else {
            // I am client, so reconnect
            if (SocketOperator::ErrorHandler(sk, SocketOperator::C_RECONNECT) == 1 ) {
                AddSocketToMonitor(sk);
            } else {
                SocketPool::GetSocketPool()->DestroySocket(sk);
            }

            return;
        }
    }

    // connect
    write_result = 0;
    if ((sk->status_ == Socket::S_CONNECTING) && (events_concern | EV_WRITE) && (events | EV_WRITE)) {
        // non-blocking connect
        // check error
        int error = 0;
        if (SocketOperator::GetSocketError(sk->fd_, error) < 0 || error != 0) {
            SocketOperator::ErrorHandler(sk, SocketOperator::C_CLOSE);
            SocketPool::GetSocketPool()->DestroySocket(sk);
            return;
        }
        // try to write
        write_result = sk->op_->WriteHandler();
        if (write_result < 0) {
            if (SocketOperator::ErrorHandler(sk, SocketOperator::C_RECONNECT) == 1 ) {
                AddSocketToMonitor(sk);
            } else {
                SocketPool::GetSocketPool()->DestroySocket(sk);
            }
            return;
        }
    }

    // accept
    int accept_result = 0;
    if ((sk->status_ == Socket::S_LISTEN) && (events_concern | EV_READ) && (events | EV_READ)) {

        accept_result = AcceptHandler();
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
        pthread_mutex_init(new_mutex, NULL);
        recv_queue_locks_.push_back(new_mutex);

        pthread_cond_t *new_cond = new pthread_cond_t;
        pthread_cond_init(new_cond, NULL);
        recv_queue_conds_.push_back(new_cond);
    }
    return 0;
}

int NetFrame::SetupSendQueues(int queue_num) {

    if (queue_num <= 0) {
        return -1;
    }

    for (int i=0; i < queue_num; i++) {

        send_queues_.push_back(list<MemBlock*>(0));

        pthread_mutex_t *new_mutex = new pthread_mutex_t;
        pthread_mutex_init(new_mutex, NULL);
        send_queue_locks_.push_back(new_mutex);
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


// return:
// -1: parameters invalid
// -2: socket exists
int NetFrame::PrepareListenSocket(std::string &my_ipstr,
                                  uint16_t my_port,
                                  Socket::SocketType type,
                                  Socket::DataFormat data_format) {

    // Check parameters
    if (my_ipstr.empty() || type != Socket::T_TCP_LISTEN) {
        return -1;
    }

    Socket *sk = SocketPool::GetSocketPool()->FindSocket(my_ipstr, my_port, type);
    if (sk) {
        return -2;
    }

    SocketOperator *op = op_factory_->CreateSocketOperator(type, data_format);
    sk = SocketPool::GetSocketPool()->CreateSocket(op);
    sk->my_ipstr_ = my_ipstr;
    sk->my_port_ = my_port;
    sk->type_ = type;
    sk->event_concern_ = Socket::EV_READ | Socket::EV_ERROR;

    // prepare socket
    if (sk->fd_ = socket(PF_INET, SOCK_STREAM, 0) < 0) {
        perror("socket() error: ");
        SocketPool::GetSocketPool()->DestroySocket(sk);
        return -3;
    }
    // Set nonblocking
    int val = fcntl(sk->fd_, F_GETFL, 0);
    if (val == -1) {
        perror("Get socket flags error: ");
        SocketPool::GetSocketPool()->DestroySocket(sk);
        return -3;
    }
    if (fcntl(sk->fd_, F_SETFL, val | O_NONBLOCK | O_NDELAY) == -1) {
        perror("Set socket flags error: ");
        SocketPool::GetSocketPool()->DestroySocket(sk);
        return -3;
    }
    // Bind address
    struct sockaddr_in listen_addr;
    //listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(sk->my_port);
    if (inet_aton(sk->my_ipstr_.c_str(), &listen_addr.sin_addr) ==0) {
        SocketPool::GetSocketPool()->DestroySocket(sk);
        return -3;
    }
    socket_len addr_len = sizeof(struct sockaddr_in);
    if (bind(sk->fd_, (struct sockaddr*)listen_addr, addr_Len) == -1) {
        SocketPool::GetSocketPool()->DestroySocket(sk);
        return -3;
    }
    // Set address reusable
    int optval = 0;
    size_t optlen = sizeof(optval);
    if (setsockopt(sk->fd_, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0) {
        perror("Set address reusable error:");
        SocketPool::GetSocketPool()->DestroySocket(sk);
        return -3;
    }
    // Listen
    if (listen(sk->fd_, 1024) == -1) {
        perror("Listen socket error: ");
        return -3;
    }
    // Asynchronously accept
    if (accept(sk->fd_, NULL, 0) == -1) {
        if (errno != EAGAIN || errno != EWOULDBLOCK) {
            perror("Async accept error: ");
            SocketPool::GetSocketPool()->DestroySocket(sk);
            return -3;
        }
    }

    // 添加一个libevent的侦听事件
    if (AddSocketToMonitor(sk) < 0) {
        SocketPool::GetSocketPool()->DestroySocket(sk);
        return -3;
    }

    return 0;
}

void NetFrame::SendQueuesNoitfyHandler(int signo, short events, void *arg) {

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
            // 添加到一个libevent的侦听事件
            if (AddSocketToMonitor(sk) < 0) {
                SocketPool::GetSocketPool()->DestroySocket(sk);
                delete pkt;
                continue;
            }
            // 成功创建新的套接口，并把数据交给它了。
        }// while
    } // for

}

};