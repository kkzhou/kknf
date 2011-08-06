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

#ifndef __UTIL_HPP_
#define __UTIL_HPP__

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <cstdio>

namespace NF {

// ����ʱ����ص����飬�����ʱ���ʽ�����ַ�������֮
// ����һ����̬��
class TimeUtil {
public:
    // ��time_t�ṹ��ʱ�䣬ת�����ַ�������2010-03-01 23:32:09
    static int TimeToString(time_t time_sec, std::string &time_str) {

        if (time_sec < 0) {
            return -1;
        }

        struct tm *plocaltime = localtime(&time_sec);
        std::vector<char> buf;
        buf.resize(100);
        size_t ret = strftime(&buf[0], buf.size(), "%Y-%m-%d %H:%M:%S", plocaltime);
        if (ret == 0) {
            return -2;
        }
        time_str.assign(&buf[0], ret);
        return 0;
    };
    // ��ȡ�ַ�����ʽ�ĵ�ǰʱ��
    static int CurrentTimeString(std::string &time_str) {

        time_t curtime = time(0);
        if (curtime == -1) {
            return -1;
        }
        int ret = TimeToString(curtime, time_str);
        return ret;
    };
    // ���ַ�����ʽ��ʱ��ת����time_t
    static int StringToTime(std::string &time_str, time_t &time_sec) {

        struct tm local_time;
        memset(&local_time, 0, sizeof(local_time));
        size_t ret = sscanf(time_str.c_str(), "%d-%d-%d %d:%d:%d",
                            &local_time.tm_year, &local_time.tm_mon, &local_time.tm_mday,
                            &local_time.tm_hour, &local_time.tm_min, &local_time.tm_sec);

        if (ret == 0U || ret != 6U) {
            return -1;
        }

        local_time.tm_year -= 1900;
        local_time.tm_mon -= 1;
        local_time.tm_isdst = 0;

        time_sec = mktime(&local_time);
        return 0;
    };

private:
    // ��ֹʹ�ã���Ϊ����һ����̬�ࡣ
    TimeUtil();
    TimeUtil(TimeUtil&);
};// class TimeUtil

#define SLOG_LEVEL 0x00000003UL

#if !defined(SLOG)
#define SLOG(level,format,arg...) \
            do { \
                if ((level) & SLOG_LEVEL) { \
                    std::string timestr; \
                    TimeUtil::CurrentTimeString(timestr); \
                    fprintf(stderr, "%lu %s %s %s %d "format, pthread_self(), timestr.c_str(), __func__, \
                            __FILE__, __LINE__, ##arg); \
                } \
            } while (0)
#endif

#define ENTERING SLOG(1, "Enter\n");
#define LEAVING SLOG(1, "Leave\n");


}; // namespace NF
#endif

