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

#include "tcp_server.hpp"

namespace ZXBNF {
 
    int TCPServer::EventCallback_For_ListenSocket(Event *e, void *arg) {
    };
    int TCPServer::EventCallback_For_DataSocket(Event *e, void *arg) {
    };
    int TCPServer::EventCallback_For_Connect(Event *e, void *arg) {
    };

    int TCPServer::TimerCallback_For_Sweep(Timer *t, void *arg) {
	
    };
    int TCPServer::TimerCallback_For_Nothing(Timer *t, void *arg) {
	ENTERING;
	int next = 5000;	// milliseconds
	LEAVING;
	return next;
    };

};
