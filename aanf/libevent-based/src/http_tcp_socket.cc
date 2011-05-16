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

#include "http_tcp_socket.h"

namespace AANF {

#define NON_PACKETIZED_BUFFER_SIZE 1048576 // 1M

HttpTcpSocket::HttpTcpSocket() {

    ENTERING;
    int ret = MemPool::GetMemPool()->GetMemBlock(NON_PACKETIZED_BUFFER_SIZE, non_packetized_buf_);
    if (ret < 0) {
        SLOG(LogLevel::L_FATAL, "can't allocate memory\n");
    }
    LEAVING;
}

HttpTcpSocket::~HttpTcpSocket() {

    ENTERING;
    int ret = MemPool::GetMemPool()->ReturnMemBlock(non_packetized_buf_);
    if (ret < 0) {
        SLOG(LogLevel::L_FATAL, "can't return the memblock\n");
    }
    LEAVING;
}


int HttpTcpSocket::ReadHandler() {

    ENTERING;
    LEAVING;
    return 1;
}

}; // namespace AANF
