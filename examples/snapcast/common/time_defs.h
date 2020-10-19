#pragma once

#include <chrono>
#include <sys/time.h>
#include <thread>

namespace chronos
{
typedef std::chrono::system_clock clk;
typedef std::chrono::time_point<clk> time_point_clk;
typedef std::chrono::seconds sec;
typedef std::chrono::milliseconds msec;
typedef std::chrono::microseconds usec;
typedef std::chrono::nanoseconds nsec;


inline static void systemtimeofday(struct timeval *tv)
{
    gettimeofday(tv, nullptr);
}

inline static void addUs(timeval& tv, int us)
{
    if (us < 0)
    {
        timeval t;
        t.tv_sec = -us / 1000000;
        t.tv_usec = (-us % 1000000);
        timersub(&tv, &t, &tv);
        return;
    }
    tv.tv_usec += us;
    tv.tv_sec += (tv.tv_usec / 1000000);
    tv.tv_usec %= 1000000;
}
//时间单位是ms。
inline static long getTickCount()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}


template <class Rep, class Period>
inline std::chrono::duration<Rep, Period> abs(std::chrono::duration<Rep, Period> d)
{
    Rep x = d.count();
    return std::chrono::duration<Rep, Period>(x >= 0 ? x : -x);
}

template <class ToDuration, class Rep, class Period>
inline int64_t duration(std::chrono::duration<Rep, Period> d)
{
    return std::chrono::duration_cast<ToDuration>(d).count();
}

template<class Rep, class Period>
inline void sleep(const std::chrono::duration<Rep, Period>& sleep_duration)
{
    std::this_thread::sleep_for(sleep_duration);
}

inline void sleep(const int32_t& ms)
{
    if(ms < 0) {
        return ;
    }
    sleep(msec(ms));
}

inline void usleep(const int32_t& us)
{
    if(us < 0) {
        return;
    }
    sleep(usec(us));
}
}
