 /*
    Copyright (C) <2011>  <ZHOU Xiaobo>

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

namespace ZXB {


class Locker {
public:
    Locker(pthread_mutex_t *mutex) {
        mutex_ = mutex;
        pthread_mutex_lock(mutex_);
    };
    ~Locker() {
        pthread_mutex_unlock(mutex_);
    };
private:
    pthread_mutex_t *mutex_;
    // Prohibits
    Locker(Locker&);
    Locker& operator=(Locker&);

};

class ConditionVariable {
public:
    ConditionVariable(pthread_mutex_t *mutex, pthread_cond_t *cond) {
        mutex_ = mutex;
        cond_ = cond;

        pthread_mutex_lock(mutex_);
    };
    ~ConditionVariable() {
        pthread_mutex_unlock(mutex_);
    };
    int Wait() {
        pthread_cond_wait(cond_, mutex_);
    };
    int Signal() {
        pthread_cond_signal(cond_);
    };
private:
    pthread_mutex_t *mutex_;
    pthread_cond_t *cond_;
    // Prohibits
    ConditionVariable(ConditionVariable&);
    ConditionVariable& operator=(ConditionVariable&);
};
};
