#include "muduo/net/EventLoopThread.h"
#include "muduo/net/EventLoop.h"
#include "mylog.h"
#include <functional>

namespace muduo
{
namespace net
{
EventLoopThread::EventLoopThread()
 : m_loop(NULL),
   m_exiting(false),
   m_thread(std::bind(&EventLoopThread::threadFunc, this)),
   m_mutex(),
   m_cond()
{

}

EventLoopThread::~EventLoopThread()
{
    m_exiting = false;
    m_loop->quit();
    m_thread.join();
}

EventLoop* EventLoopThread::startLoop()
{
    m_thread.start();
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_loop == NULL) {
            m_cond.wait(lock);
        }
    }
    return m_loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_loop = &loop;
        m_cond.notify_one();
    }
    loop.loop();
}

} // namespace net

} // namespace muduo

