
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
