#include "TimerQueue.h"
#include "mylog.h"
#include <string.h>

#include <sys/timerfd.h>
#include "Timer.h"
#include "EventLoop.h"
#include "TimerId.h"

namespace muduo
{
namespace detail
{

int createTimerFd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
    if(timerfd < 0) {
        myloge("create timerfd fail");
        exit(1);
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t us = when.usValue() - Timestamp::now().usValue();
    if(us < 100) {
        us = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(us/Timestamp::kUsPerSecond);
    ts.tv_nsec = static_cast<long>((us%Timestamp::kUsPerSecond) *1000);
    return ts;
}


void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    if(n != sizeof(howmany)) {
        myloge("read fail");
    }
}
void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof(newValue));
    memset(&oldValue, 0, sizeof(oldValue));
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if(ret) {
        myloge("timerfd_settime fail");
        exit(1);
    }
}
} // namespace detail

} // namespace muduo


namespace muduo
{
namespace net
{

TimerQueue::TimerQueue(EventLoop *loop)
 : m_loop(loop),
   m_timerfd(muduo::detail::createTimerFd()),
   m_timerfdChannel(loop, m_timerfd),
   m_timers(),
   m_callingExpiredTimers(false)
{
    m_timerfdChannel.setReadCallback(
        std::bind(&TimerQueue::handleRead, this)
    );
    m_timerfdChannel.enableReading();
}
TimerQueue::~TimerQueue()
{
    ::close(m_timerfd);
    for(auto it = m_timers.begin(); it!=m_timers.end(); it++) {
        delete it->second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
    Timestamp when,
    double interval
)
{
    Timer *timer = new Timer(cb, when, interval);
    m_loop->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer)
    );
    return TimerId(timer);

}
void TimerQueue::addTimerInLoop(Timer *timer)
{
    m_loop->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if(earliestChanged) {
        muduo::detail::resetTimerfd(m_timerfd, timer->expiration());
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    auto it = m_timers.begin();
    if(it == m_timers.end() || when < it->first) {
        earliestChanged = true;
    }
    std::pair<TimerList::iterator, bool> result = m_timers.insert(std::make_pair(when, timer));
    std::pair<ActiveTimerSet::iterator, bool> result1 = m_activeTimers.insert(ActiveTimer(timer, timer->sequence()));
    return earliestChanged;
}


void TimerQueue::handleRead()
{
    m_loop->assertInLoopThread();
    Timestamp now(Timestamp::now());
    muduo::detail::readTimerfd(m_timerfd, now);
    std::vector<Entry> expired = getExpired(now);
    m_callingExpiredTimers = true;
    m_cancelingTimers.clear();
    for(auto it=expired.begin(); it!= expired.end(); it++) {
        it->second->run();
    }
    m_callingExpiredTimers = false;
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator it = m_timers.lower_bound(sentry);
    std::copy(m_timers.begin(), it, std::back_inserter(expired));
    m_timers.erase(m_timers.begin(), it);

    for(auto it=expired.begin(); it!=expired.end(); it++) {
        ActiveTimer timer(it->second, it->second->sequence());
        size_t n = m_activeTimers.erase(timer);
    }
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;
    for(auto it = expired.begin(); it!=expired.end(); it++) {
        ActiveTimer timer(it->second, it->second->sequence());

        if(it->second->repeat()
            && (m_cancelingTimers.find(timer) == m_cancelingTimers.end())) {
            it->second->restart(now);
            insert(it->second);
        } else {
            delete it->second;
        }
    }
    if(!m_timers.empty()) {
        nextExpire = m_timers.begin()->second->expiration();
    }
    if(nextExpire.valid()) {
        muduo::detail::resetTimerfd(m_timerfd, nextExpire);
    }
}


void TimerQueue::cancel(TimerId timerId)
{
    m_loop->runInLoop(
        std::bind(&TimerQueue::cancelInLoop, this, timerId)
    );
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    m_loop->assertInLoopThread();
    //因为TimerQueue是Timer的友元，所以可以直接在这里访问Timer的成员变量。
    ActiveTimer timer(timerId.m_timer, timerId.m_sequence);
    ActiveTimerSet::iterator it = m_activeTimers.find(timer);
    if(it != m_activeTimers.end()) {
        //找到了。
        size_t n = m_timers.erase(Entry(it->first->expiration(), it->first));
        delete it->first;
        m_activeTimers.erase(it);

    } else if(m_callingExpiredTimers) {
        m_cancelingTimers.insert(timer);
    }

}


} // namespace net

} // namespace muduo

