#ifndef _MUDUO_TIMER_ID_H_
#define _MUDUO_TIMER_ID_H_

#include "muduo/base/copyable.h"
namespace muduo
{
namespace net
{
class Timer ;

class TimerId : copyable
{
public:
    explicit TimerId(Timer* timer= NULL, int64_t sequence=0)
     : m_timer(timer),
       m_sequence(sequence)
    {

    }
    friend class TimerQueue;
private:
    Timer* m_timer;
    int64_t m_sequence;
};

} // namespace net

} // namespace muduo


#endif //_MUDUO_TIMER_ID_H_
