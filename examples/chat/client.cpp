#include "mylog.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/Connector.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/net/EventLoopThread.h"
#include <unistd.h>
#include "muduo/net/TcpClient.h"
#include "codec.h"
#include <mutex>
#include <iostream>

using namespace muduo;
using namespace muduo::net;

class ChatClient : muduo::noncopyable
{
public:
    ChatClient(EventLoop* loop, const InetAddress& addr)
    : m_client(loop, addr),
      m_codec(
          std::bind(&ChatClient::onStringMessage, this, _1, _2, _3)
      )
    {
        m_client.setConnectionCallback(
            std::bind(&ChatClient::onConnection, this, _1)
        );
        m_client.setMessageCallback(
            std::bind(&LengthHeaderCodec::onMessage, &m_codec, _1, _2, _3)
        );
        m_client.enableRetry();
    }
    void connect()
    {
        m_client.connect();
    }
    void disconnect()
    {
        m_client.disconnect();
    }

    void write(const muduo::StringPiece& message) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_connection) {
            m_codec.send(m_connection.get(), message);
        }
    }
private:
    void onConnection(const TcpConnectionPtr & conn) {
        if(conn->connected()) {
            m_connection = conn;
        } else {
            m_connection.reset();
        }
    }
    void onStringMessage(const TcpConnectionPtr& conn,
        const std::string& message,
        muduo::Timestamp receiveTime)
    {
        printf("[%s]>> %s\n", receiveTime.toFormattedString().c_str(), message.c_str());
    }
    TcpClient m_client;
    LengthHeaderCodec m_codec;
    std::mutex m_mutex;
    TcpConnectionPtr m_connection;
};
int main(int argc, char const *argv[])
{
    mylogd("client");
    if(argc > 2) {
        EventLoopThread loopThread;
        uint16_t port = (uint16_t)(std::atoi(argv[2]));
        InetAddress serverAddr(argv[1], port);
        ChatClient client(loopThread.startLoop(), serverAddr);
        client.connect();
        std::string line;
        while(std::getline(std::cin, line)) {
            client.write(line);
        }
        client.disconnect();
        sleep(1);
    } else {
        printf("usage:%s ip port\n", argv[0]);
    }
    return 0;
}
