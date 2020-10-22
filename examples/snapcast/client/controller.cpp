#include "controller.h"
#include "mylog.h"
#include "common/utils.h"
#include "common/message/hello.h"

Controller::Controller(EventLoop* loop, const InetAddress& addr)
: m_client(loop, addr)
{
    m_instance = 1;

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

void Controller::write(msg::BaseMessage* message) {
    // std::unique_lock<std::mutex> lock(m_mutex);
    //消息得到的是一个ostream。需要把这个ostream转成string
    // std::ostream stream;
    // membuf databuf;
    if(m_connection) {
        // m_connection->send(message);
    }
}

void Controller::onConnection(const TcpConnectionPtr & conn) {
    if(conn->connected()) {
        m_connection = conn;
        //连接建立后，发送hello消息
        //拿到mac地址
        std::string macAddress = ::getMacAddress(m_connection->getSocketFd());
        //把mac地址作为hostId
        m_hostId = ::getHostId(macAddress);
        msg::Hello hello(macAddress, m_hostId, m_instance);

        // m_connection->send()
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
