#include "muduo/net/TcpClient.h"
#include "mylog.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/Connector.h"
#include "muduo/net/SocketOps.h"
namespace muduo
{
namespace detail
{
void removeCononection(muduo::net::EventLoop *loop,
    const muduo::net::TcpConnectionPtr &conn)
{
    loop->queueInLoop(
        std::bind(&muduo::net::TcpConnection::connectDestroyed, conn)
    );
}
void removeConnector(const muduo::net::ConnectorPtr& connector)
{

}
} // namespace detail

} // namespace muduo


namespace muduo
{
namespace net
{
TcpClient::TcpClient(EventLoop *loop, const InetAddress& serverAddr)
 : m_loop(loop),
   m_connector(new Connector(loop, serverAddr)),
   m_retry(false),
   m_connect(false),
   m_nextConnId(1)
{
    m_connector->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, _1)
    );

}

TcpClient::~TcpClient()
{
    TcpConnectionPtr conn;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        conn = m_connection;
    }
    if(conn ) {
        CloseCallback cb = std::bind(&muduo::detail::removeCononection, m_loop, _1);
        m_loop->runInLoop(
            std::bind(&TcpConnection::setCloseCallback, conn, cb)
        );

    } else {
        m_connector->stop();
        m_loop->runAfter(1, std::bind(
            &muduo::detail::removeConnector, m_connector
        ));
    }
}

void TcpClient::connect()
{
    m_connect = true;
    m_connector->start();
}

void TcpClient::disconnect()
{
    m_connect = false;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_connection) {
            m_connection->shutdown();
        }
    }
}

void TcpClient::stop()
{
    m_connect = false;
    m_connector->stop();
}

void TcpClient::newConnection(int sockfd)
{
    m_loop->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toIpPort().c_str(), m_nextConnId);
    m_nextConnId++;
    std::string connName = buf;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    TcpConnectionPtr conn(
        new TcpConnection(
            m_loop,
            connName,
            sockfd,
            localAddr,
            peerAddr
        )
    );
    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, _1)
    );
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_connection = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    m_loop->assertInLoopThread();
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_connection.reset();
    }
    m_loop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
    if(m_retry && m_connect) {
        mylogd("reconnecting");
        m_connector->restart();
    }
}

} // namespace net

} // namespace muduo

