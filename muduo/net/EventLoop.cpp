#include "EventLoop.h"
#include "mylog.h"
#include "muduo/base/CurrentThread.h"
#include <sys/poll.h>
#include <stdlib.h>
#include "muduo/net/Poller.h"
#include "muduo/net/Channel.h"
#include "muduo/net/TimerQueue.h"
#include <sys/eventfd.h>
#include <signal.h>

namespace muduo
{
namespace net
{
const int kPollTimeMs = 10*1000;
__thread EventLoop * t_loopInThisThread = 0;

class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};
IgnoreSigPipe initObj;

static int  createEventfd()
{
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(fd < 0) {
        myloge("create eventfd fail");
        abort();
    }
    mylogd("eventfd:%d", fd);
    return fd;
}

EventLoop::EventLoop()
 : m_looping(false),
   m_callingPendingFunctors(false),
   m_threadId(muduo::CurrentThread::tid()),
   m_quit(false),
   m_poller(new Poller(this)),
   m_timerQueue(new TimerQueue(this)),
   m_wakeupFd(createEventfd()),
   m_wakeupChannel(new Channel(this, m_wakeupFd)),
   m_pollReturnTime()
{
    mylogd("EventLoop created in %d", m_threadId);
    if(t_loopInThisThread) {
        myloge("another EventLoop exist in %d this thread", m_threadId);
        exit(1);
    } else {
        t_loopInThisThread = this;
    }
    m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
    m_wakeupChannel->enableReading();
}


EventLoop::~EventLoop()
{
    ::close(m_wakeupFd);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    m_looping = true;
    m_quit = false;
    while(!m_quit) {
        m_activeChannels.clear();
        m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_activeChannels);
        for(auto it = m_activeChannels.begin(); it!=m_activeChannels.end(); it++) {
            (*it)->handleEvent(m_pollReturnTime);
        }
        doPendingFunctors();
    }
}


void EventLoop::abortNotInLoopThread()
{
    myloge(".");
    exit(1);
}


void EventLoop::updateChannel(Channel* channel)
{
    m_poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    m_poller->removeChannel(channel);
}

void EventLoop::quit()
{
    m_quit = true;
    if(!isInLoopThread()) {
        wakeup();
    }
}



TimerId EventLoop::runAt(const Timestamp &time, const TimerCallback& cb)
{
    return m_timerQueue->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback &cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time,cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return m_timerQueue->addTimer(cb, time, interval);
}


void EventLoop::runInLoop(const Functor& cb)
{
    if(isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pendingFunctors.push_back(cb);
    }
    if(!isInLoopThread() || m_callingPendingFunctors) {
        wakeup();
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(m_wakeupFd, &one, sizeof(one));
    if(n != sizeof(one)) {
        myloge("write eventfd fail");
        //abort();
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }
    m_callingPendingFunctors = true;
    for(size_t i=0; i<functors.size(); i++) {
        functors[i]();
    }
    m_callingPendingFunctors = false;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(m_wakeupFd, &one, sizeof(one));
    if(n != sizeof(one)) {
        myloge("read eventfd fail");
    }
}



void EventLoop::cancel(TimerId timerId)
{
    m_timerQueue->cancel(timerId);
}
} // namespace net

} // namespace muduo

