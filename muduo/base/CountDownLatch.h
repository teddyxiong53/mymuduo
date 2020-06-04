#ifndef _MUDUO_COUNT_DOWN_LATCH_H_
#define _MUDUO_COUNT_DOWN_LATCH_H_

#include "muduo/base/noncopyable.h"
#include <mutex>
#include <condition_variable>

namespace muduo
{
class CountDownLatch : noncopyable
{
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();
    int getCount();
    
private:
    int m_count;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};

} // namespace muduo


#endif
