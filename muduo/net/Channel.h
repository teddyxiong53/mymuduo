#ifndef _MUDUO_CHANNEL_H_
#define _MUDUO_CHANNEL_H_

#include <memory>
#include "muduo/base/noncopyable.h"
#include <functional>
#include "muduo/base/Timestamp.h"

namespace muduo
{
namespace net
{
class EventLoop;
using muduo::Timestamp;

class Channel: noncopyable
{
public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void(muduo::Timestamp)> ReadEventCallback;
    Channel(EventLoop *loop, int fd);
    ~Channel();
    void handleEvent(Timestamp receiveTime);

    void setReadCallback(const ReadEventCallback& cb) {
        m_readCallback = cb;
    }
    void setWriteCallback(const EventCallback& cb) {
        m_writeCallback = cb;
    }
    void setErrorCallback(const EventCallback& cb) {
        m_errorCallback = cb;
    }
    void setCloseCallback(const EventCallback& cb) {
        m_closeCallback = cb;
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
    void enableWriting() {
        m_events |= kWriteEvent;
        update();
    }
    void disableWriting() {
        m_events &= ~kWriteEvent;
        update();
    }
    bool isWriting() {
        return m_events & kWriteEvent;
    }
    void disableAll() {
        m_events = kNoneEvent;
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
    void tie(const std::shared_ptr<void>& obj);
    void remove();
private:
    void handleEventWithGuard(Timestamp receiveTime);
    void update();
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *m_loop;
    const int m_fd;
    int m_events;
    int m_revents;
    int m_index;
    bool m_eventHandling;

    std::weak_ptr<void> m_tie;
    bool m_tied;
    bool m_addedToLoop;

    ReadEventCallback m_readCallback;
    EventCallback m_writeCallback;
    EventCallback m_errorCallback;
    EventCallback m_closeCallback;
};


} // namespace net

} // namespace muduo



#endif //_MUDUO_CHANNEL_H_
