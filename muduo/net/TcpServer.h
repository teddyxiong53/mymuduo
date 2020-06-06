#ifndef _MUDUO_TCP_CONNECTOIN_H_
#define _MUDUO_TCP_CONNECTOIN_H_

#include "muduo/base/noncopyable.h"
#include "muduo/net/InetAddress.h"
#include <string>
#include <vector>
#include <memory>
#include "muduo/net/Callbacks.h"
#include <map>
namespace muduo
{
namespace net
{
class Acceptor;
class EventLoop;
class TcpConnection;
class EventLoopThreadPool;

class TcpServer: muduo::noncopyable {
public:
    TcpServer(
        EventLoop *loop,
        const InetAddress& listenAddr
    );
    ~TcpServer();

    void start();
    void setThreadNum(int numThreads);

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        m_connectionCallback = cb;
    }
    void setMessageCallback(const MessageCallback& cb)
    {
        m_messageCallback = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        m_writeCompleteCallback = cb;
    }

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
    EventLoop *m_loop;
    const std::string m_name;
    std::unique_ptr<Acceptor> m_acceptor;
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    bool m_started;
    int m_nextConnId;
    ConnectionMap m_connections;
    std::unique_ptr<EventLoopThreadPool> m_threadPool;
};

} // namespace net

} // namespace muduo


#endif
