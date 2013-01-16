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

#include "server.hpp"

namespace ZXBNF {

    int UDPEvent::EventCallback() {
	
    }
 
    int Server::AddTCPLVListenSocket(int index, char *ipstr, unsigned short hport) {
    }
    int Server::AddTCPLVClientSocket(int index, char *ipstr, unsigned short hport) {
    }

    int Server::AddUDPServerSocket(char *ipstr, unsigned short hport) {
    }
    int Server::AddUDPClientSocket(char *ipstr, unsigned short hport) {
    }
    int Server::OnSocketError(int socket) {
    }
    int Server::OnSocketReadable(int socket) {
    }
    int Server::OnSocketWritable(int socket) {
    }
};
