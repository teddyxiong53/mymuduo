#include "Thread.h"
#include "muduo/base/CurrentThread.h"

#include <sys/prctl.h>

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
    typedef muduo::Thread::ThreadFunc ThreadFunc;
    ThreadFunc func;
    std::string name;
    pid_t* tid;//这2个是指针。
    CountDownLatch *latch;

    ThreadData(ThreadFunc f, const std::string& n, pid_t *t, CountDownLatch* l)
    {
        func = f;
        name = n;
        tid = t;
        latch = l;
    }
    void runInThread()
    {
        *tid = muduo::CurrentThread::tid();
        tid = NULL;
        latch->countDown();
        latch = NULL;

        muduo::CurrentThread::t_threadName = name.empty() ? "muduoThread" : name.c_str();
        prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName);

    }
};


void *startThread(void *obj)
{
    ThreadData *data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

} // namespace detail

std::atomic<int> Thread::m_numCreated;

Thread::Thread(ThreadFunc func, const std::string& name)
 : m_started(false),
   m_joined(false),
   m_pthreadId(0),
   m_tid(0),
   m_func(std::move(func)),
   m_name(name),
   m_latch(1)
{
    setDefaultName();
}
void Thread::setDefaultName()
{
    int num = m_numCreated.load();
    if(m_name.empty()) {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        m_name = buf;
    }
}

Thread::~Thread()
{
    if(m_started && !m_joined) {
        pthread_detach(m_pthreadId);
    }
}

void Thread::start()
{
    m_started = true;
    detail::ThreadData *data = new detail::ThreadData(m_func, m_name, &m_tid, &m_latch);
    pthread_t pth = pthread_create(&m_pthreadId, NULL, &detail::startThread, data);

}
} // namespace muduo

