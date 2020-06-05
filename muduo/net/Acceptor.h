#ifndef _MUDUO_ACCEPTOR_H_
#define _MUDUO_ACCEPTOR_H_
#include "muduo/base/noncopyable.h"
#include "muduo/net/Channel.h"
#include "muduo/net/Socket.h"


namespace muduo
{
namespace net
{
class EventLoop;
class InetAddress;

class Acceptor: muduo::noncopyable
{
public:
    typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;
    Acceptor(EventLoop *loop, const InetAddress& listenAddr);

    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        m_newConnectionCallback = cb;
    }
    bool listening() {
        return m_listening;
    }
    void listen();
private:
    void handleRead();
    EventLoop* m_loop;
    Socket m_acceptSocket;
    Channel m_acceptChannel;
    bool m_listening;
    NewConnectionCallback m_newConnectionCallback;
};

} // namespace net

} // namespace muduo

#endif //_MUDUO_ACCEPTOR_H_
