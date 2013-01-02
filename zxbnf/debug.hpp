#ifndef __DEBGUG_H__
#define __DEBGUG_H__

#include <fcntl.h>

namespace ZXBNF {

class Logger {
public:
    Logger(int fd, int socket = -1) {

	log_file_fd_ = fd;
	log_socket_ = socket;

	buffer_len_ = 2 * 1024 * 1024;
	buffer = malloc(buffer_len_);
	start_ = end_ = 0;

	remote_log_ = false;
	if (socket >= 0) {
	    remote_log_ = true;
	}
	pthread_mutex_init(&mutex_, 0, 0);
	pthread_cond_init(&cond_, 0, 0);
    };

private:
    friend void WriteLogThreadProc(void *arg) {

	Logger *logger = reinterpreter_cast<Logger*>(arg);
	assert(logger->log_file_fd_ >= 0)
	assert(logger->log_socket_ >= 0);

	while (true) {
	    pthread_mutex_lock(&mutex_);
	    while (start_ == end_) {
		pthread_cond_wait(*&cond_, &mutex_);
	    }

	    if (end_ < start_) {
		// rewind
		struct iovec iov[2];
		iov[0].iov_base = buffer_ + start_;
		iov[0].iov_len = buffer_len_ - start_;
		iov[1].iov_base = buffer_;
		iov[1].iov_len = end_;

		ret = writev(log_file_fd_, iov, 2);
		if (ret < 0) {
		    if (errno != EAGAIN) {
		    }
		} else if (ret == 0) {
		} else {
		    if (buffer_len_ - start_ > ret) {
			start_ += ret;
		    } else {
			start_ = ret - (bufferlen_ - start_);
		    }
		}
	    }
	    int ret = write(buffer_ + 
	}
	
    };

    char *buffer_;
    unsigned int buffer_len_;
    unsigned int start_;
    unsigned int end_;
    
    int log_file_fd_;
    int log_socket_;

    bool remote_log_;

    bool use_thread_;
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;
};

};

#endif
