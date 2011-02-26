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

#include "sync_sender_receiver.h"
#include "memblock.h"


namespace AANF {

SyncSR::SyncSR(bool is_tcp) {

    // prepare socket
    is_tcp_ = is_tcp;
    if (fd_ = socket(PF_INET, is_tcp_ ? SOCK_STREAM : SOCK_DGRAM, 0) < 0) {
        perror("socket() error: ");
        return -3;
    }
}

SyncSR::~SyncSR() {
    CloseSR()();
}

int SyncSR::CloseSR() {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
    return 0;
}

int SetOpterationTimeout(int usec) {

    if (usec < 0) {
        return -1;
    } else if (usec == 0) {
        return 0;
    } else {
    }

    int ret = 0;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_usec;
    socklen_t = timeout_len = sizeof(struct timeval);
    ret = setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, &timeout_len);
    if (ret == -1) {
        perror("setsockopt error:");
        return -2;
    }
    ret = setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &timeout, &timeout_len);
    if (ret == -1) {
        perror("setsockopt error:");
        return -2;
    }
    return 0;
}

int SyncSR::ConnectToServer(std::string server_ip, uint16_t server_port) {
       // Check parameters
    if (server_ip.empty()) {
        return -1;
    }

    if (fd_ >= 0) {
        close(fd_);
    }
    server_ipstr_ = server_ip;
    server_port_ = server_port;

    struct sockaddr_in server_addr;
    //server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_aton(server_ip.c_str(), &server_addr.sin_addr) ==0) {
        return -3;
    }
    socket_len addr_len = sizeof(struct sockaddr_in);

    // connect
    if (connect(fd_, (struct sockaddr*)&server_addr, addr_len) == -1) {
        if (errno != EAGAIN || errno != EWOULDBLOCK) {
            perror("Connect error: ");
            return -3;
        }
    }
    return 0;

}

int BinSyncSR::SyncSendRecv(MemBlock *to_send, MemBlock *&to_recv) {

    if (!to_send || timeout_usec < 0) {
        return -1;
    }

    int ret = 0;
    //开始发送
    int byte_sent_num = 0;
    byte_sent_num = send(fd_, to_send->start_ +
                         byte_sent_num, to_send->used_ - byte_sent_num, 0);

    if (byte_sent_num != to_send->used_) {
        return -2;
    }
    // 开始接收
    // 先接收长度域
    int byte_recv_num = 0;
    uint32_t len_field = 0;
    byte_recv_num = recv(fd_, (char*)&len_field, sizeof(uint32_t), 0);
    if (byte_recv_num != sizeof(uint32_t)) {
        return -3;
    }

    uint32_t real_len = ntohl(len_field);
    to_recv = MemPool::GetMemPool()->GetMemBlock(real_len);
    if (to_recv == 0) {
        return -4;
    }
    memcpy(to_recv->start_, &len_field, 4);
    to_recv->used_ += 4;

    // 然后接收数据域
    byte_recv_num = 0;
    byte_recv_num = recv(fd_, to_recv->start_, to_recv->used_ , 0);

    if (byte_recv_num != real_len) {
        MemPool::GetMemPool()->ReturnMemBlock(to_recv);
        to_recv = 0;
        return -5;
    }
    return 0;
}



int LineSyncSR::SyncSendRecv(MemBlock *to_send, MemBlock *&to_recv) {

    if (!to_send || timeout_usec < 0) {
        return -1;
    }

    //开始发送
    int byte_sent_num = 0;
    byte_sent_num = send(fd_, to_send->start_, to_send->used_, 0);

    if (byte_sent_num != to_send->used_) {
        return -2;
    }
    // 开始接收
    int byte_recv_num = 0;
    to_recv = MemPool::GetMemPool()->GetMemBlock(1024);
    if (to_recv == 0) {
        return -3;
    }

    // 然后接收数据域
    byte_recv_num = 0;
    byte_recv_num = recv(fd_, to_recv->start_, to_recv->used_ , 0);
    if (byte_recv_num <= 0) {
        MemPool::GetMemPool()->ReturnMemBlock(to_recv);
        to_recv = 0;
        return -4;
    }

    char *pos = to_recv->start_;
    while (pos - to_recv->start_ < to_recv->used_ && *pos != '\n') {

    }

    if (*pos != '\n') {
        MemPool::GetMemPool()->ReturnMemBlock(to_recv);
        to_recv = 0;
        return -5;
    }

    return 0;
}
}
