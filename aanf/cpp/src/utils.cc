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

#include <pthread.h>
#include <string>
#include "utils.h"
#include <sys/time.h>

namespace AANF {


Locker::Locker(pthread_mutex_t *mutex) {
    mutex_ = mutex;
    pthread_mutex_lock(mutex_);
}

Locker::~Locker() {
    pthread_mutex_unlock(mutex_);
}

Locker::Locker(Locker&){
}

Locker::Locker& operator=(Locker&){
}

ConditionVariable::ConditionVariable(pthread_mutex_t *mutex, pthread_cond_t *cond) {
    mutex_ = mutex;
    cond_ = cond;

    pthread_mutex_lock(mutex_);
}

ConditionVariable::~ConditionVariable() {
    pthread_mutex_unlock(mutex_);
}

int ConditionVariable::Wait() {
    pthread_cond_wait(cond_, mutex_);
}

int ConditionVariable::Signal() {
    pthread_cond_signal(cond_);
}


uint32_t g_log_level = 0;
uint32_t SetLogLevel(uint32_t new_level) {
    uint32_t old_level = g_log_level;
    g_log_level = new_level;
    return old_level;
}



int TimeUtil::TimeToString(time_t time_sec, std::string &time_str) {

    if (time_sec == -1) {
        return -1;
    }

    struct tm *plocaltime = localtime(&time_sec);
    vector<char> buf;
    buf.reserve(100);
    size_t ret = strftime(&buf[0], buf.size(), "%Y-%m-%d %H:%M:%S", plocaltime);
    if (ret == 0) {
        return -2;
    }
    time_str.assign(&buf[0], ret);
    return 0;
}

std::string TimeUtil::CurrentTimeString(std::string &time_str) {

    time_t curtime = time(NULL);
    if (curtime == -1) {
        return -1;
    }
    int ret = TimeToString(curtime, time_str);
    return ret;
}

time_t TimeUtil::StringToTime(std::string &time_str) {

    struct tm local_time;
    memset(&local_time, 0, sizeof(local_time));
    size_t ret = sscanf(time_str.c_str(), "%d-%d-%d %d:%d:%d",
                        &local_time.tm_year, &local_time.tm_mon, &local_time.tm_mday,
                        &local_time.tm_hour, &local_time.tm_min, &local_time.tm_sec);

    if (ret == EOF || ret != 6) {
        return -1;
    }
    local_time.tm_year -= 1900;
    local_time.tm_mon -= 1;
    local_time.tm_isdst = 0;

    return mktime(&local_time);
}

TimeUtil::TimeUtil(){
}
TimeUtil::TimeUtil(TimeUtil&){
}

}; // namespace ZXB
