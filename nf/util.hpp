#ifndef __UTIL_HPP__
#define __UTIL_HPP__

namespace NF {

#define SLOG_LEVEL 0xFFFFFFFFUL

#if !defined(SLOG)
#define SLOG(level,format,arg...)					\
    do {								\
	if ((level) & SLOG_LEVEL) {					\
	    std::string timestr;					\
	    TimeUtil::CurrentTimeString(timestr);			\
	    fprintf(stderr, "%lu %s %s %s %d "format, pthread_self(),	\
		    timestr.c_str(), __func__,				\
		    __FILE__, __LINE__, ##arg);				\
	}								\
    } while (0)
#endif

#define ENTERING SLOG(1, "Enter\n");
#define LEAVING SLOG(1, "Leave\n");
#define LEAVING2 SLOG(1, "Leave on error\n");


    class Time {
    public:
	static int TimeToString(time_t time_sec, std::string &time_str) {
	    
	    ENTERING;
	    if (time_sec < 0) {
		LEAVING2;
		return -1;
	    }

	    struct tm *plocaltime = localtime(&time_sec);
	    std::vector<char> buf;
	    buf.resize(100);
	    size_t ret = strftime(&buf[0], buf.size(), "%Y-%m-%d %H:%M:%S", plocaltime);
	    if (ret == 0) {
		LEAVING2;
		return -2;
	    }
	    time_str.assign(&buf[0], ret);
	    LEAVING;
	    return 0;
	};

	static int CurrentTimeString(std::string &time_str) {

	    ENTERING;
	    time_t curtime = time(0);
	    if (curtime == -1) {
		LEAVING2;
		return -1;
	    }
	    int ret = TimeToString(curtime, time_str);
	    LEAVING;
	    return ret;
	};

	static int StringToTime(std::string &time_str, time_t &time_sec) {

	    ENTERING;
	    struct tm local_time;
	    memset(&local_time, 0, sizeof(local_time));
	    size_t ret = sscanf(time_str.c_str(), "%d-%d-%d %d:%d:%d",
				&local_time.tm_year, &local_time.tm_mon, &local_time.tm_mday,
				&local_time.tm_hour, &local_time.tm_min, &local_time.tm_sec);

	    if (ret == 0U || ret != 6U) {
		LEAVING2;
		return -1;
	    }

	    local_time.tm_year -= 1900;
	    local_time.tm_mon -= 1;
	    local_time.tm_isdst = 0;

	    time_sec = mktime(&local_time);
	    LEAVING;
	    return 0;
	};

	inline unsigned long long second() { return second_; };
	inline unsigned long long microsecond() { return microsecond_; };
	inline void set_second(unsigned long long second) { second_ = second; };
	inline void set_microsecond(unsigned long long microsecond) { 
	    microsecond_ = microsecond; 
	};
    private:
	unsigned long long second_;
	unsigned long long microsecond_;
    };

};
#endif
