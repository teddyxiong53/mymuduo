#include "muduo/net/TcpServer.h"
#include "mylog.h"
#include "muduo/net/Acceptor.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/SocketOps.h"

#include "muduo/net/Callbacks.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/net/EventLoopThreadPool.h"

namespace muduo
{
namespace net
{

TcpServer::TcpServer(
    EventLoop* loop,
    const InetAddress& listenAddr
)
 : m_loop(loop),
   m_name(listenAddr.toIpPort()),
   m_acceptor(new Acceptor(loop, listenAddr)),
   m_started(false),
   m_nextConnId(1),
   m_threadPool(new EventLoopThreadPool(loop))
{
    m_acceptor->setNewConnectionCallback(std::bind(
        &TcpServer::newConnection, this, _1, _2
    ));

}

TcpServer::~TcpServer()
{

}

void TcpServer::setThreadNum(int numThreads)
{
    m_threadPool->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if(!m_started) {
        m_started = true;
        m_threadPool->start();
    }
    if(!m_acceptor->listening()) {
        m_loop->runInLoop(
            std::bind(&Acceptor::listen, m_acceptor.get())
        );
    }
}

void TcpServer::newConnection(
    int sockfd,
    const InetAddress& peerAddr
)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", m_nextConnId);
    m_nextConnId ++;
    std::string connName = m_name + buf;

    mylogd("new connection from: %s", peerAddr.toIpPort().c_str());
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    EventLoop *ioloop = m_threadPool->getNextLoop();
    //然后创建一个TCPConnection
    TcpConnectionPtr conn(
        new TcpConnection(ioloop, m_name, sockfd, localAddr, peerAddr)
    );
    m_connections[connName] = conn;
    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, _1)
    );

    ioloop->runInLoop(
        std::bind(&TcpConnection::connectEstablished, conn)
    );
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    m_loop->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    mylogd("remove connection");
    m_loop->assertInLoopThread();
    size_t n = m_connections.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}
} // namespace net

} // namespace muduo

