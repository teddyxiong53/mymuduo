#include "Channel.h"
#include "mylog.h"
#include "EventLoop.h"
#include <poll.h>

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
   m_index(-1)
{

}

void Channel::handleEvent()
{
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
            m_readCallback();
        }
    }
    if(m_revents & POLLOUT) {
        if(m_writeCallback) {
            m_writeCallback();
        }
    }
}

void Channel::update()
{
    m_loop->updateChannel(this);
}


} // namespace net

} // namespace muduo


