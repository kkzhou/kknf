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
#include <sys/time.h>
#include <cstdio>
#include "utils.h"

namespace AANF {


Locker::Locker(pthread_mutex_t *mutex) {
    ENTERING;
    mutex_ = mutex;
    pthread_mutex_lock(mutex_);
    LEAVING;
}

Locker::~Locker() {
    ENTERING;
    pthread_mutex_unlock(mutex_);
    LEAVING;
}

Locker::Locker(Locker&){
    ENTERING;
    LEAVING;
}

Locker::Locker& operator=(Locker&){
    ENTERING;
    LEAVING;
}

ConditionVariable::ConditionVariable(pthread_mutex_t *mutex, pthread_cond_t *cond) {
    ENTERING;
    mutex_ = mutex;
    cond_ = cond;

    pthread_mutex_lock(mutex_);
    LEAVING;
}

ConditionVariable::~ConditionVariable() {
    ENTERING;
    pthread_mutex_unlock(mutex_);
    LEAVING;
}

int ConditionVariable::Wait() {
    ENTERING;
    pthread_cond_wait(cond_, mutex_);
    LEAVING;
}

int ConditionVariable::Signal() {
    ENTERING;
    pthread_cond_signal(cond_);
    LEAVING;
}

uint32_t SLog::slog_level = 0xFFFFFFFF;
FILE *SLog::slog_fd_ = stderr;

SLog::InitSLog(uint32_t log_level, string &logfile) {

    log_level = log_level;

    if (logfile.empty()) {
        slog_fd_ = stderr;
    }

    slog_fd_ = fopen(filename.c_str(), "a+");
}


uint32_t SLog::SetLogLevel(uint32_t new_level) {

    uint32_t old_level = log_level;
    log_level = new_level;
    return old_level;
}

int SLog::SetSLogFileDescriptor(std::string &filename) {

    if (filename.empty()) {
        slog_fd_ = stderr;
        return -1;
    }

    slog_fd_ = fopen(filename.c_str(), "a+");
    if (slog_fd_ == NULL) {
        return -2;
    }
    return 0;
}


int TimeUtil::TimeToString(time_t time_sec, std::string &time_str) {

    ENTERING;
    if (time_sec < 0) {
        SLOG(LogLevel::L_LOGICERR, "time_sec < 0\n")
        LEAVING;
        return -1;
    }

    struct tm *plocaltime = localtime(&time_sec);
    vector<char> buf;
    buf.reserve(100);
    size_t ret = strftime(&buf[0], buf.size(), "%Y-%m-%d %H:%M:%S", plocaltime);
    if (ret == 0) {
        SLOG(LogLevel::L_SYSERR, "strftime error\n");
        LEAVING;
        return -2;
    }
    time_str.assign(&buf[0], ret);
    LEAVING;
    return 0;
}

std::string TimeUtil::CurrentTimeString(std::string &time_str) {

    ENTERING;
    time_t curtime = time(NULL);
    if (curtime == -1) {
        SLOG(LogLevel::L_SYSERR, "time() error\n");
        LEAVING;
        return -1;
    }
    int ret = TimeToString(curtime, time_str);
    LEAVING;
    return ret;
}

time_t TimeUtil::StringToTime(std::string &time_str) {

    ENTERING;
    struct tm local_time;
    memset(&local_time, 0, sizeof(local_time));
    size_t ret = sscanf(time_str.c_str(), "%d-%d-%d %d:%d:%d",
                        &local_time.tm_year, &local_time.tm_mon, &local_time.tm_mday,
                        &local_time.tm_hour, &local_time.tm_min, &local_time.tm_sec);

    if (ret == EOF || ret != 6) {
        SLOG(LogLevel::L_LOGICERR, "time_str invalid\n");
        return -1;
    }

    local_time.tm_year -= 1900;
    local_time.tm_mon -= 1;
    local_time.tm_isdst = 0;

    LEAVING;
    return mktime(&local_time);
}

TimeUtil::TimeUtil(){
    ENTERING;
    LEAVING;
}
TimeUtil::TimeUtil(TimeUtil&){
    ENTERING;
    LEAVING;
}

}; // namespace ZXB
