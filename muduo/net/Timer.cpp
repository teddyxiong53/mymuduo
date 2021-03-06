#include "Timer.h"
#include "muduo/base/Timestamp.h"

namespace muduo
{
namespace net
{
std::atomic<int64_t> Timer::s_numCreated;

void Timer::restart(Timestamp now)
{
    if(m_repeat) {
        m_expiration = addTime(now, m_interval);
    } else {
        m_expiration = Timestamp::invalid();
    }
}
} // namespace net

} // namespace muduo

