#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/EventLoopThreadPool.h"

#include "mylog.h"

namespace muduo
{
namespace net
{

EventLoopThreadPool::EventLoopThreadPool(
    EventLoop* baseLoop
)
 : m_baseLoop(baseLoop),
   m_started(false),
   m_numThreads(0),
   m_next(0)
{

}
EventLoopThreadPool::~EventLoopThreadPool()
{

}
void EventLoopThreadPool::start()
{
    m_baseLoop->assertInLoopThread();
    m_started = true;
    for(int i=0; i<m_numThreads; i++) {
        EventLoopThread *t = new EventLoopThread();
        m_threads.push_back(t);
        m_loops.push_back(t->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    m_baseLoop->assertInLoopThread();
    EventLoop * loop = m_baseLoop;
    if(!m_loops.empty()) {
        loop = m_loops[m_next];
        m_next ++;
        if(m_next >= m_loops.size()) {
            m_next = 0;
        }
    }
    return loop;
}

} // namespace net

} // namespace muduo


