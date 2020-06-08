#ifndef _MUDUO_TIMER_ID_H_
#define _MUDUO_TIMER_ID_H_

#include "muduo/base/copyable.h"
#include "mylog.h"

namespace muduo
{
namespace net
{
class Timer ;

class TimerId : copyable
{
public:
    TimerId()
     : m_timer(NULL),
       m_sequence(0)
    {

    }
    TimerId(Timer* timer, int64_t sequence=0)
     : m_timer(timer),
       m_sequence(sequence)
    {
        mylogd("add timer:%lld", m_sequence);
    }
    int64_t sequence() const {
        return m_sequence;
    }
    friend class TimerQueue;
private:
    Timer* m_timer;
    int64_t m_sequence;
};

} // namespace net

} // namespace muduo


#endif //_MUDUO_TIMER_ID_H_
