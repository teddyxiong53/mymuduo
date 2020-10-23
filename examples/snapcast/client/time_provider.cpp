#include "time_provider.h"
#include "mylog.h"

TimeProvider::TimeProvider()
 : m_diffToServer(0)
{
    m_diffBuffer.setSize(200);
}

void TimeProvider::setDiff(const tv& c2s, const tv& s2c)
{
    //转成ms数
    double diff = ((double)c2s.sec/2.0 - (double)s2c.sec/2.0) * 1000
            +
            ((double)c2s.usec/2.0 - (double)s2c.usec/2.0) / 1000;
    setDiffToServer(diff);
}

void TimeProvider::setDiffToServer(double ms)
{
    static int32_t lastTimeSync = 0;
    timeval now;
    chronos::systemtimeofday(&now);
    //如果上次更新是1分钟之前，那么清空buffer
    if(!m_diffBuffer.empty() && (std::abs(now.tv_sec - lastTimeSync) > 60)) {
        mylogd("last time sync is 60s ago");
        m_diffToServer = ms*1000;
        m_diffBuffer.clear();
    }
    lastTimeSync = now.tv_sec;
    m_diffBuffer.add(ms*1000);
    m_diffToServer = m_diffBuffer.median(3);
}
