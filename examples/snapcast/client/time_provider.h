#pragma once
#include "double_buffer.h"
#include "common/time_defs.h"
#include "common/message/message.h"
#include <atomic>
#include <chrono>

class TimeProvider
{
public:
    static TimeProvider& getInstance()
    {
        static TimeProvider instance;
        return instance;
    }
    void setDiffToServer(double ms);
    void setDiff(const tv& c2s, const tv& s2c);

    template<typename T>
    inline T getDiffToServer()
    {
        return std::chrono::duration_cast<T>(chronos::usec(m_diffToServer));
    }
    template <typename T>
    static T sinceEpoche(const chronos::time_point_clk &point)
    {
        return std::chrono::duration_cast<T>(point.time_since_epoch());
    }
    static chronos::time_point_clk toTimePoint(const tv& timeval)
    {
        return chronos::time_point_clk(chronos::usec(timeval.usec) + chronos::sec(timeval.sec));
    }
    inline static chronos::time_point_clk now()
    {
        return chronos::clk::now();
    }
    inline static chronos::time_point_clk serverNow()
    {
        return chronos::clk::now() + TimeProvider::getInstance().getDiffToServer<chronos::usec>();
    }
private:
    TimeProvider();
    TimeProvider(TimeProvider const&);//不要实现
    void operator=(TimeProvider const&);
    DoubleBuffer<chronos::usec::rep> m_diffBuffer;
    std::atomic<chronos::usec::rep> m_diffToServer;
};
