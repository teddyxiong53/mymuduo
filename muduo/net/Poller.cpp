#include "Poller.h"
#include "Channel.h"
#include "mylog.h"

#include <poll.h>

namespace muduo
{
namespace net
{
Poller::Poller(EventLoop* loop)
 : m_loop(loop)
{

}
Poller::~Poller()
{

}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::poll(&*m_pollfds.begin(), m_pollfds.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if(numEvents > 0) {
        mylogd("%d events happens", numEvents);
        fillActiveChannels(numEvents, activeChannels);
    } else if(numEvents == 0) {
        //mylogd("nothing happens");
    } else {
        myloge("poll error");
    }
    return now;
}

void Poller::fillActiveChannels(
    int numEvents,
    ChannelList* activeChannels
)
{
    auto it = m_pollfds.begin();
    for(; it != m_pollfds.end() && numEvents>0; it++) {
        if(it->revents > 0) {
            numEvents --;
            mylogd("fd:%d", it->fd);
            auto ch = m_channels.find(it->fd);
            Channel *channel = ch->second;
            channel->set_revents(it->revents);
            activeChannels->push_back(channel);
        }
    }
}


void Poller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    mylogd("add to fd:%d events:%x", channel->fd(), channel->events());
    if(channel->index() < 0) {
        //说明是新的。
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        m_pollfds.push_back(pfd);
        int idx = static_cast<int>(m_pollfds.size()) - 1;
        channel->set_index(idx);
        m_channels[pfd.fd] = channel;
    } else {
        //说明是修改当前已经有的。
        int idx = channel->index();
        struct pollfd& pfd = m_pollfds[idx];
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if(channel->isNoneEvent()) {
            pfd.fd = -channel->fd() -1;
        }
    }
}

void Poller::removeChannel(Channel* channel)
{
    mylogd("remove fd:%d", channel->fd());
    int idx = channel->index();
    //const struct pollfd& pfd = m_pollfds[idx]
    size_t n = m_channels.erase(channel->fd());
    if(idx == m_pollfds.size() - 1) {
        m_pollfds.pop_back();//刚好是移除最后一个。
    } else {
        int channelAtEnd = m_pollfds.back().fd;
        std::iter_swap(m_pollfds.begin()+idx, m_pollfds.end()-1);
        if(channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd - 1;
        }
        m_channels[channelAtEnd]->set_index(idx);
        m_pollfds.pop_back();
    }
}
} // namespace net

} // namespace muduo

