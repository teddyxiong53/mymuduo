#ifndef _MUDUO_TCP_CONNECTION_H_
#define _MUDUO_TCP_CONNECTION_H_
#include <memory>
#include "muduo/net/InetAddress.h"
#include "muduo/net/Callbacks.h"
#include "muduo/base/noncopyable.h"
#include "muduo/net/Buffer.h"

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
    void setCloseCallback(const CloseCallback &cb)
    {
        m_closeCallback = cb;
    }
    void setTcpNoDelay(bool on);
    void setWriteCompleteCallback(const WriteCompleteCallback cb)
    {
        m_writeCompleteCallback = cb;
    }
    void connectEstablished();
    void connectDestroyed();
    void send(const void *data, int len);
    void send(const StringPiece& message);
    void send(Buffer* buf);
    void shutdown();
    int getSocketFd();
private:
    enum StateE {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected,
    };
    void setState(StateE s) {
        m_state = s;
    }
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void *data, size_t len);
    void shutdownInLoop();

    EventLoop *m_loop;
    std::string m_name;
    StateE m_state;
    std::unique_ptr<Socket> m_socket;
    std::unique_ptr<Channel> m_channel;
    InetAddress m_localAddr;
    InetAddress m_peerAddr;
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    CloseCallback m_closeCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    muduo::net::Buffer m_inputBuffer;
    muduo::net::Buffer m_outputBuffer;
};


} // namespace net

} // namespace muduo



#endif //_MUDUO_TCP_CONNECTION_H_
