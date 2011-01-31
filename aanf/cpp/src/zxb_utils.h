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
#include <stdio.h>

namespace ZXB {

// 一个简单的Logger，用宏来控制log级别，并预定义了格式，即
// "级别” “时间” “函数” “文件” “代码行” “自定义内容”
enum LogLevel {
    L_FATAL = 1,
    L_SYSERR = 2,
    L_LOGICERR = 4,
    L_INFO = 8,
    L_DEBUG = 16,
};
#if !defined(SLOG)
#define SLOG(level,format,arg...) \
            do { \
                if ((level) & g_log_level) { \
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
                    fprintf(stderr, "%s %s %s %s %d "#format, level_str.c_str(), \
                            timestr.c_str(), __PRETTY_FUNCTION__, \
                            __FILE__, __LINE__, ##arg); \
                } \
            } while (0)
#endif
// 全局的日志级别，通过控制它的值，可以控制哪些日志打印，哪些不打印。
extern uint32_t g_log_level;
uint32_t SetLogLevel(uint32_t new_loevel);

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
    static int TimeToString(time_t time_sec, std::string &time_str);

    static std::string CurrentTimeString(std::string &time_str);

    static time_t StringToTime(std::string &time_str);

private:
    TimeUtil();
    TimeUtil(TimeUtil&);
};// class TimeUtil

}; // namespace ZXB
