#include "EventLoop.h"
#include "mylog.h"
#include "muduo/base/CurrentThread.h"
#include <sys/poll.h>
#include <stdlib.h>
#include "muduo/net/Poller.h"
#include "muduo/net/Channel.h"

namespace muduo
{
namespace net
{
const int kPollTimeMs = 10*1000;
__thread EventLoop * t_loopInThisThread = 0;
EventLoop::EventLoop()
 : m_looping(false),
   m_threadId(muduo::CurrentThread::tid()),
   m_quit(false),
   m_poller(new Poller(this))
{
    mylogd("EventLoop created in %d", m_threadId);
    if(t_loopInThisThread) {
        myloge("another EventLoop exist in %d this thread", m_threadId);
        exit(1);
    } else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop()
{
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    m_looping = true;
    m_quit = false;
    while(!m_quit) {
        m_activeChannels.clear();
        m_poller->poll(kPollTimeMs, &m_activeChannels);
        for(auto it = m_activeChannels.begin(); it!=m_activeChannels.end(); it++) {
            (*it)->handleEvent();
        }
    }
}


void EventLoop::abortNotInLoopThread()
{
    myloge(".");
    exit(1);
}


void EventLoop::updateChannel(Channel* channel)
{
    m_poller->updateChannel(channel);
}

void EventLoop::quit()
{
    m_quit = true;
}

} // namespace net

} // namespace muduo

