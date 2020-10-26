#include "Channel.h"
#include "mylog.h"
#include "EventLoop.h"
#include <poll.h>
#include <assert.h>

namespace muduo
{
namespace net
{

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
 : m_loop(loop),
   m_fd(fd),
   m_events(0),
   m_revents(0),
   m_index(-1),
   m_eventHandling(false),
   m_tied(false),
   m_addedToLoop(false)
{

}
Channel::~Channel()
{
    assert(!m_eventHandling);
}

void Channel::handleEvent(Timestamp receivedTime)
{
    std::shared_ptr<void> guard;
    if(m_tied) {
        guard = m_tie.lock();
        if(guard) {
            handleEventWithGuard(receivedTime);
        }
    } else {
        handleEventWithGuard(receivedTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    m_eventHandling = true;
    if(m_revents & POLLNVAL) {
        mylogd("POLLNVAL");
    }
    if(m_revents & (POLLERR | POLLNVAL)) {
        if(m_errorCallback) {
            m_errorCallback();
        }
    }
    if(m_revents & (POLLIN | POLLPRI | POLLRDHUP)) {
        if(m_readCallback) {
            m_readCallback(receiveTime);
        }
    }
    if(m_revents & POLLOUT) {
        if(m_writeCallback) {
            m_writeCallback();
        }
    }
    m_eventHandling = false;
}

void Channel::update()
{
    m_addedToLoop = true;
    m_loop->updateChannel(this);
}

void Channel::remove()
{
    m_addedToLoop = false;
    m_loop->removeChannel(this);
}
void Channel::tie(const std::shared_ptr<void>& obj)
{
    m_tie = obj;
    m_tied = true;
}

} // namespace net

} // namespace muduo


