#ifndef _MUDUO_TIEMR_H_
#define _MUDUO_TIEMR_H_

#include <memory>
#include "muduo/base/noncopyable.h"
#include <functional>
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include <atomic>

using muduo::Timestamp;
namespace muduo
{

namespace net
{
class Timer : noncopyable
{
public:

    Timer(const TimerCallback& cb, Timestamp when, double interval)
     : m_callback(cb),
       m_expiration(when),
       m_interval(interval),
       m_repeat(interval>0.0),
       m_sequence(++s_numCreated)
    {

    }
    void run() {
        m_callback();
    }
    Timestamp expiration() {
        return m_expiration;
    }
    bool repeat() {
        return m_repeat;
    }
    void restart(Timestamp now) ;
    int64_t sequence() const {
        return m_sequence;
    }
private:
    const TimerCallback m_callback;
    Timestamp m_expiration;
    const double m_interval;
    const bool m_repeat;
    static std::atomic<int64_t> s_numCreated;
    const int64_t m_sequence;
};

} // namespace net


} // namespace muduo


#endif //_MUDUO_TIEMR_H_
