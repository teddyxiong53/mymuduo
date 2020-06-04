#ifndef _MUDUO_EVENTLOOP_H_
#define _MUDUO_EVENTLOOP_H_

#include "muduo/base/noncopyable.h"
#include <unistd.h>
#include "muduo/base/CurrentThread.h"

namespace muduo
{
namespace net
{

class EventLoop : noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void assertInLoopThread() {
        if(!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() {
        return m_threadId == muduo::CurrentThread::tid();
    }
private:
    void abortNotInLoopThread();
    bool m_looping;
    const pid_t m_threadId;
};



} // namespace net

} // namespace muduo

#endif //_MUDUO_EVENTLOOP_H_
