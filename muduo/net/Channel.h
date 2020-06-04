#ifndef _MUDUO_CHANNEL_H_
#define _MUDUO_CHANNEL_H_

#include <memory>
#include "muduo/base/noncopyable.h"
#include <functional>

namespace muduo
{
namespace net
{
class EventLoop;
class Channel: noncopyable
{
public:
    typedef std::function<void()> EventCallback;
    Channel(EventLoop *loop, int fd);

    void handleEvent();
    void setReadCallback(const EventCallback& cb) {
        m_readCallback = cb;
    }
    void setWriteCallback(const EventCallback& cb) {
        m_writeCallback = cb;
    }
    void setErrorCallback(const EventCallback& cb) {
        m_errorCallback = cb;
    }

    int fd() {
        return m_fd;
    }
    int events() {
        return m_events;
    }
    int set_revents(int revt) {
        m_revents = revt;
    }
    bool isNoneEvent() {
        return m_events == kNoneEvent;
    }
    void enableReading() {
        m_events |= kReadEvent;
        update();
    }
    int index() {
        return m_index;
    }
    void set_index(int index) {
        m_index = index;
    }
    EventLoop *ownerLoop() {
        return m_loop;
    }
private:
    void update();
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *m_loop;
    const int m_fd;
    int m_events;
    int m_revents;
    int m_index;

    EventCallback m_readCallback;
    EventCallback m_writeCallback;
    EventCallback m_errorCallback;
};


} // namespace net

} // namespace muduo



#endif //_MUDUO_CHANNEL_H_
