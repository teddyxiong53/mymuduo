#ifndef _MUDUO_TCP_CONNECTION_H_
#define _MUDUO_TCP_CONNECTION_H_
#include <memory>
#include "muduo/net/InetAddress.h"
#include "muduo/net/Callbacks.h"
#include "muduo/base/noncopyable.h"

namespace muduo
{
namespace net
{
class EventLoop;
class Channel;
class Socket;

class TcpConnection : muduo::noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, const std::string& name,
        int sockfd, const InetAddress& localAddr,
        const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const {
        return m_loop;
    }
    const std::string& name() const {
        return m_name;
    }
    const InetAddress& localAddress()  {
        return m_localAddr;
    }
    const InetAddress& peerAddress() {
        return m_peerAddr;
    }
    bool connected() const {
        return m_state == kConnected;
    }
    void setConnectionCallback(const ConnectionCallback &cb)
    {
        m_connectionCallback = cb;
    }
    void setMessageCallback(const MessageCallback& cb)
    {
        m_messageCallback = cb;
    }
    void connectEstablished();

private:
    enum StateE {
        kConnecting,
        kConnected,
    };
    void setState(StateE s) {
        m_state = s;
    }
    void handleRead();
    EventLoop *m_loop;
    std::string m_name;
    StateE m_state;
    std::unique_ptr<Socket> m_socket;
    std::unique_ptr<Channel> m_channel;
    InetAddress m_localAddr;
    InetAddress m_peerAddr;
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
};


} // namespace net

} // namespace muduo



#endif //_MUDUO_TCP_CONNECTION_H_
