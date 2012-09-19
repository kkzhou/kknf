#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "socket_info.h"
#include "timer.h"

int process_socket_event(struct socket_info *info);
int process_timer_event(struct timer *timer);


#endif
