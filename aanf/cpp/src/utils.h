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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <pthread.h>
#include <stdio.h>
#include <string>
#include <cstdio>

namespace AANF {

// 一个简单的日志(SLOG)，用宏来控制log级别，并预定义了格式，即
// "级别” “时间” “函数” “文件” “代码行” “自定义内容”
enum LogLevel {
    L_FATAL = 1,
    L_SYSERR = 2,
    L_LOGICERR = 4,
    L_INFO = 8,
    L_DEBUG = 16,
    L_FUNC = 32 // 打印每个函数的调用序列
};

#if !defined(SLOG)
#define SLOG(level,format,arg...) \
            do { \
                if ((level) & SLog::slog_level) { \
                    enum LogLevel tmp_level = level; \
                    std::string level_str; \
                    if (tmp_level == L_FATAL) { \
                        level_str = "FATAL"; \
                    } else if (tmp_level == L_SYSERR) { \
                        level_str = "SYSERR"; \
                    } else if (tmp_level == L_LOGICERR) { \
                        level_str = "LOGICERR"; \
                    } else if (tmp_level == L_INFO) { \
                        level_str = "INFO"; \
                    } else if (tmp_level == L_DEBUG) { \
                        level_str = "DEBUG"; \
                    } else { \
                        break; \
                    } \
                    std::string timestr; \
                    CurrentTimeString(timestr); \
                    fprintf(stderr, "%s %s %s %s %d ", level_str.c_str(), \
                            timestr.c_str(), __PRETTY_FUNCTION__, \
                            __FILE__, __LINE__); \
                    fprintf(stderr, format, ##arg); \
                } \
            } while (0)
#endif

#define ENTERING SLOG(LogLevel::L_FUNC, "Enter\n");
#define LEAVING SLOG(LogLevel::L_FUNC, "Leave\n");

// 全局的日志级别，通过控制它的值，可以控制哪些日志打印，哪些不打印。
class SLog {
public:
    static InitSLog(uint32_t log_level = 0xFFFFFFFF, std::string &logfile = "");
    static uint32_t slog_level;
    static FILE *slog_fd_;
    static uint32_t SetLogLevel(uint32_t new_loevel);
    static int SetSLogFileDescriptor(std::string &filename);
};


// 封装了pthread_mutex_t的加锁和解锁过程，通过构造函数和析构函数来控制，进而通过作用域控制。
class Locker {
public:
    Locker(pthread_mutex_t *mutex);
    ~Locker();
private:
    pthread_mutex_t *mutex_;
    // Prohibits
    Locker(Locker&);
    Locker& operator=(Locker&);

};


// 封装了条件变量的操作过程，通过构造和析构来获取和释放锁，从而可以用作用域来控制。
// 需要访问条件变量时，先定一个对象，
// 该对象构造时就获取锁，然后判断条件，如果不满足则调用wait。
// 生产者会调用Signal来唤醒wait。
class ConditionVariable {
public:
    ConditionVariable(pthread_mutex_t *mutex, pthread_cond_t *cond);
    ~ConditionVariable();
    int Wait();
    int Signal();
private:
    pthread_mutex_t *mutex_;
    pthread_cond_t *cond_;
    // Prohibits
    ConditionVariable(ConditionVariable&);
    ConditionVariable& operator=(ConditionVariable&);
};


// 处理时间相关的事情，例如把时间格式化成字符串，或反之
// 这是一个静态类
class TimeUtil {
public:
    // 把time_t结构的时间，转换成字符串，如2010-03-01 23:32:09
    static int TimeToString(time_t time_sec, std::string &time_str);
    // 获取字符串形式的当前时间
    static std::string CurrentTimeString(std::string &time_str);
    // 把字符串形式的时间转换成time_t
    static time_t StringToTime(std::string &time_str);

private:
    // 禁止使用，因为这是一个静态类。
    TimeUtil();
    TimeUtil(TimeUtil&);
};// class TimeUtil

}; // namespace ZXB
#endif
