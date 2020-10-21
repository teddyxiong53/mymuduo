#include "controller.h"
#include "mylog.h"


Controller::Controller(EventLoop* loop, const InetAddress& addr)
: m_client(loop, addr)
{
    m_client.setConnectionCallback(
        std::bind(&Controller::onConnection, this, _1)
    );
    m_client.setMessageCallback(
        std::bind(&Controller::onMessage, this, _1, _2, _3)
    );
    m_client.enableRetry();
}
void Controller::connect()
{
    m_client.connect();
}
void Controller::disconnect()
{
    m_client.disconnect();
}

void Controller::write(const muduo::StringPiece& message) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_connection) {
        m_connection->send(message);
    }
}
void Controller::onConnection(const TcpConnectionPtr & conn) {
    if(conn->connected()) {
        m_connection = conn;
    } else {
        m_connection.reset();
    }
}
void Controller::onMessage(const TcpConnectionPtr& conn,
    Buffer* buf,
    muduo::Timestamp receiveTime)
{
    printf("[%s]>> %s\n", receiveTime.toFormattedString().c_str(), buf->retrieveAsString().c_str());
}
