#ifndef _MUDUO_TIMER_QUEUE_H_
#define _MUDUO_TIMER_QUEUE_H_

#include "muduo/base/noncopyable.h"
#include <vector>
#include <map>
#include <set>

#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/Channel.h"

using muduo::Timestamp;

namespace muduo
{
namespace net
{
class EventLoop;
class Timer;
class TimerId;

class TimerQueue: noncopyable
{
public:
    TimerQueue(EventLoop *loop);
    ~TimerQueue();
    TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval );
    void cancel(TimerId timerId);

private:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    void handleRead();
    void addTimerInLoop(Timer* timer);
    bool insert(Timer *timer);
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);
    void cancelInLoop(TimerId timerId);

    EventLoop* m_loop;
    const int m_timerfd;
    Channel m_timerfdChannel;
    TimerList m_timers;

    //针对cancel增加
    bool m_callingExpiredTimers;
    ActiveTimerSet m_activeTimers;
    ActiveTimerSet m_cancelingTimers;

};

} // namespace net

} // namespace muduo


#endif //_MUDUO_TIMER_QUEUE_H_
