#ifndef _MUDUO_THREAD_H_
#define _MUDUO_THREAD_H_

#include "muduo/base/noncopyable.h"
#include <pthread.h>
#include <memory>
#include <functional>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <atomic>
#include "CountDownLatch.h"

namespace muduo
{

class Thread : noncopyable
{
public:
    typedef std::function<void()> ThreadFunc;
    explicit Thread(ThreadFunc, const std::string& name);
    ~Thread();
    void start();
    int join();
    bool started() const {
        return m_started;
    }
    pid_t tid() const {
        return m_tid;
    }
    const std::string& name() const {
        return m_name;
    }
    static int numCreated() {
        return m_numCreated.load(std::memory_order_relaxed);
    }
private:
    bool m_started;
    bool m_joined;
    pthread_t m_pthreadId;
    pid_t m_tid;
    ThreadFunc m_func;
    std::string m_name;
    CountDownLatch m_latch;
    static std::atomic<int> m_numCreated;
};
} // namespace muduo


#endif
