#ifndef _MUDUO_EVENTLOOP_H_
#define _MUDUO_EVENTLOOP_H_

#include "muduo/base/noncopyable.h"
#include <unistd.h>
#include "muduo/base/CurrentThread.h"
#include <vector>
#include <memory>
#include "TimerId.h"
#include "muduo/net/Callbacks.h"
#include <mutex>

#include "muduo/base/Timestamp.h"
using muduo::Timestamp;

namespace muduo
{
namespace net
{
class Channel;
class Poller;
class TimerQueue;

class EventLoop : noncopyable
{
public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void assertInLoopThread() {
        if(!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() {
        return m_threadId == muduo::CurrentThread::tid();
    }

    void updateChannel(Channel* channel);

    TimerId runAfter(double delay, const TimerCallback& cb);
    TimerId runAt(const Timestamp &time,const TimerCallback &cb);
    TimerId runEvery(double interval, const TimerCallback& cb);

    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);

    void wakeup();

private:
    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();

    bool m_callingPendingFunctors;

    int m_wakeupFd;
    std::unique_ptr<Channel> m_wakeupChannel;
    std::vector<Functor> m_pendingFunctors;
    std::mutex m_mutex;//保护m_pendingFunctors
    bool m_looping;
    const pid_t m_threadId;
    typedef std::vector<Channel*> ChannelList;
    std::unique_ptr<Poller> m_poller;
    ChannelList m_activeChannels;
    std::unique_ptr<TimerQueue> m_timerQueue;

    bool m_quit;
};



} // namespace net

} // namespace muduo

#endif //_MUDUO_EVENTLOOP_H_
