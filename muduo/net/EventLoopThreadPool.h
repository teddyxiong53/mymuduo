#ifndef _EVENT_LOOP_THREAD_POOL_H_
#define _EVENT_LOOP_THREAD_POOL_H_
#include "muduo/base/noncopyable.h"
#include <memory>
#include <vector>

namespace muduo
{
namespace net
{
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : muduo::noncopyable
{
public:
    EventLoopThreadPool(EventLoop *baseLoop);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) {
        m_numThreads = numThreads;
    }
    void start();
    EventLoop* getNextLoop();
    
private:
    EventLoop *m_baseLoop;
    bool m_started;
    int m_numThreads;
    int m_next;
    std::vector<EventLoopThread*> m_threads;
    std::vector<EventLoop*> m_loops;
};

} // namespace net

} // namespace muduo



#endif //_EVENT_LOOP_THREAD_POOL_H_
