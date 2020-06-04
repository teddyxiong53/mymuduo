#include "Thread.h"
#include "CurrentThread.h"


namespace muduo
{
namespace detail
{

void afterFork()
{
    muduo::CurrentThread::t_cachedTid = 0;
    muduo::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
}
class ThreadNameInitializer
{
public:
    ThreadNameInitializer() {
        muduo::CurrentThread::t_threadName = "main";
        CurrentThread::tid();
        pthread_atfork(NULL, NULL, &afterFork);
    }
};

ThreadNameInitializer init;//静态构造。

struct ThreadData {

};

} // namespace detail




} // namespace muduo

