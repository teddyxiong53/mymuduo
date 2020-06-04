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
   m_timers()
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
    m_loop->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if(earliestChanged) {
        muduo::detail::resetTimerfd(m_timerfd, timer->expiration());
    }
    return TimerId(timer);
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
    return earliestChanged;
}


void TimerQueue::handleRead()
{
    m_loop->assertInLoopThread();
    Timestamp now(Timestamp::now());
    muduo::detail::readTimerfd(m_timerfd, now);
    std::vector<Entry> expired = getExpired(now);
    for(auto it=expired.begin(); it!= expired.end(); it++) {
        it->second->run();
    }
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator it = m_timers.lower_bound(sentry);
    std::copy(m_timers.begin(), it, std::back_inserter(expired));
    m_timers.erase(m_timers.begin(), it);
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;
    for(auto it = expired.begin(); it!=expired.end(); it++) {
        if(it->second->repeat()) {
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

} // namespace net

} // namespace muduo

