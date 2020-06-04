#ifndef _MUDUO_POLLER_H_
#define _MUDUO_POLLER_H_

#include <memory>
#include "muduo/base/noncopyable.h"
#include <vector>
#include <map>
#include "muduo/net/EventLoop.h"
#include "muduo/base/Timestamp.h"

struct pollfd;
namespace muduo
{
namespace net
{
class Channel;


class Poller : noncopyable
{
public:
    typedef std::vector<Channel*> ChannelList;
    Poller(EventLoop *loop);
    ~Poller();

    Timestamp poll(int timeoutMs, ChannelList * activeChannels);

    void updateChannel(Channel *channel);
    void assertInLoopThread() {
        m_loop->assertInLoopThread();
    }
private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels);

    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;
    EventLoop *m_loop;
    PollFdList m_pollfds;
    ChannelMap m_channels;

};


} // namespace net

} // namespace muduo


#endif //_MUDUO_POLLER_H_
