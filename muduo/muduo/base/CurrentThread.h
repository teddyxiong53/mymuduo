#ifndef MUDUO_BASE_CURRENT_THREAD_H
#define MUDUO_BASE_CURRENT_THREAD_H

#include <muduo/base/Types.h>

namespace muduo
{
namespace CurrentThread
{
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char *t_threadName;

inline int tid()
{
    if(t_cachedTid == 0) {
        cacheTid();
    }
    return t_cachedTid;
}

inline const char *tidString()
{
    return t_tidString;
}

inline int tidStringLength()
{
    return t_tidStringLength;
}

inline const char *name()
{
    return t_threadName;
}

bool isMainThread();
void sleepUsec(int64_t usec);
string stackTrace(bool demangle);
} // CurrentThread

} // muduo


#endif //MUDUO_BASE_CURRENT_THREAD_H