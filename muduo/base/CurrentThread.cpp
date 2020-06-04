#include "CurrentThread.h"
#include <cxxabi.h>
#include <type_traits>
#include <stdio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

namespace muduo
{

namespace CurrentThread
{
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char * t_threadName = "unknown";

static_assert(std::is_same<int, pid_t>::value, "pid_t should be the same as int");

bool isMainThread()
{
    return tid() == getpid();
}


void cacheTid()
{
    if(t_cachedTid == 0) {
        t_cachedTid = gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString), "%5d ", t_cachedTid);
    }
}


} // namespace CurrentThread

} // namespace muduo
