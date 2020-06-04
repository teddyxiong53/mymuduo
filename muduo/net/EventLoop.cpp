#include "EventLoop.h"
#include "mylog.h"
#include "muduo/base/CurrentThread.h"
#include <sys/poll.h>
#include <stdlib.h>

namespace muduo
{
namespace net
{
__thread EventLoop * t_loopInThisThread = 0;
EventLoop::EventLoop()
 : m_looping(false),
   m_threadId(muduo::CurrentThread::tid())
{
    mylogd("EventLoop created in %d", m_threadId);
    if(t_loopInThisThread) {
        myloge("another EventLoop exist in %d this thread", m_threadId);
        exit(1);
    } else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop()
{
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    m_looping = true;
    ::poll(NULL, 0, 3*1000);
    mylogd("stop looping");
    m_looping = false;
}


void EventLoop::abortNotInLoopThread()
{
    myloge(".");
    exit(1);
}

} // namespace net

} // namespace muduo

