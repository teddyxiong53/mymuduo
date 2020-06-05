#ifndef _MUDUO_EVENT_LOOP_THREAD_H_
#define _MUDUO_EVENT_LOOP_THREAD_H_

#include "muduo/base/noncopyable.h"
#include "muduo/base/Thread.h"
#include <mutex>
#include <condition_variable>

namespace muduo
{
namespace net
{
class EventLoop;
class EventLoopThread : noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop *startLoop();


private:
    void threadFunc();
    EventLoop* m_loop;
    bool m_exiting;
    muduo::Thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};
} // namespace net

} // namespace muduo



#endif //_MUDUO_EVENT_LOOP_THREAD_H_
