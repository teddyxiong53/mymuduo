#ifndef MUDUO_BASE_COUNTDOWNLATCH_H
#define MUDUO_BASE_COUNTDOWNLATCH_H

#include <muduo/base/Condtion.h>
#include <muduo/base/Mutex.h>

namespace muduo
{
class CountDownLatch: noncopyable
{
public:
    CountDownLatch(int count);
    void wait();
    void countDown();
    int getCount() const;

private:
    Condition condition_;
    mutable MutexLock mutex_;
    int count_;
};
} // muduo


#endif //MUDUO_BASE_COUNTDOWNLATCH_H