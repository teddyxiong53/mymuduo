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
    explicit TimerId(Timer* timer)
     : m_value(timer)
    {

    }
private:
    Timer* m_value;
};

} // namespace net

} // namespace muduo


#endif //_MUDUO_TIMER_ID_H_
