#ifndef _MUDUO_TCP_CLIETN_H_
#define _MUDUO_TCP_CLIETN_H_

#include "muduo/base/noncopyable.h"
#include <memory>
#include "muduo/net/TcpConnection.h"
#include "muduo/net/Callbacks.h"
#include <mutex>


namespace muduo
{
namespace net
{
class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;
class TcpClient: muduo::noncopyable
{
public:
    TcpClient(EventLoop* loop, const InetAddress& serverAddr);
    ~TcpClient();
    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const {
        return m_connection;
    }
    bool retry() const {
        return m_retry;
    }
    void enableRetry() {
        m_retry  = true;
    }
    void setConnectionCallback(const ConnectionCallback &cb)
    {
        m_connectionCallback = cb;
    }
    void setMessageCallback(const MessageCallback &cb)
    {
        m_messageCallback = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        m_writeCompleteCallback = cb;
    }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop * m_loop;
    ConnectorPtr m_connector;
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    bool m_retry;
    bool m_connect;
    int m_nextConnId;
    std::mutex m_mutex;
    TcpConnectionPtr m_connection;
};


} // namespace net

} // namespace muduo


#endif
