#ifndef MUDUO_BASE_MUTEX_H
#define MUDUO_BASE_MUTEX_H

#include <muduo/base/CurrentThread.h>
#include <muduo/base/noncopyable.h>

#include <assert.h>
#include <pthread.h>

namespace muduo
{
class MutexLock: public noncopyable
{
public:
    MutexLock()
     : holder_(0) {
        pthread_mutex_init(&mutex_,NULL);
    }
    ~MutexLock() {
        pthread_mutex_destroy(&mutex_);
    }
    bool isLockedByThisThread() {
        return holder_ == CurrentThread::tid();
    }

    void lock() {
        pthread_mutex_lock(&mutex_);
        assignHolder();
    }
    void unlock() {
        unassignHolder();
        pthread_mutex_unlock(&mutex_);

    }
    pthread_mutex_t * getPthreadMutex() {
        return &mutex_;
    }

private:
    friend class Condtion;
    class UnassignGuard:noncopyable
    {
    public:
        explicit UnassignGuard(MutexLock& owner)
         : owner_(owner)
        {
            owner_.unassignHolder();
        }
        ~UnassignGuard() {
            owner_.assignHolder();
        }
    private:
        MutexLock& owner_;
    };
    void assignHolder() {
        holder_ = CurrentThread::tid();
    }
    void unassignHolder() {
        holder_ = 0;
    }

    pid_t holder_;
    pthread_mutex_t mutex_;
};

class MutexLockGuard: noncopyable
{
public:
    MutexLockGuard(MutexLock& mutex)
     : mutex_(mutex)
    {
        mutex_.lock();
    }
    ~MutexLockGuard() {
        mutex_.unlock();
    }
private:
    MutexLock& mutex_;
};
} // muduo


#endif //MUDUO_BASE_MUTEX_H