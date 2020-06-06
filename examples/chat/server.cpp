#include "mylog.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/net/InetAddress.h"
#include <stdlib.h>
#include <set>
#include "codec.h"
#include "muduo/net/Callbacks.h"
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

class ChatServer : muduo::noncopyable
{
public:
    ChatServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress& addr)
     : m_server(loop, addr),
       m_codec(
           std::bind(&ChatServer::onStringMessage, this, _1, _2, _3)
       )
    {
        m_server.setConnectionCallback(
            std::bind(&ChatServer::onConnection, this, _1)
        );
        m_server.setMessageCallback(
            std::bind(&LengthHeaderCodec::onMessage, &m_codec, _1, _2, _3)
        );
    }
    void start() {
        m_server.start();
    }

private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn) {
        if(conn->connected()) {
            m_connections.insert(conn);
        } else {
            m_connections.erase(conn);
        }
    }
    void onStringMessage(const muduo::net::TcpConnectionPtr& conn, const std::string& message, muduo::Timestamp receiveTime) {
        for(auto it=m_connections.begin(); it!= m_connections.end(); it++) {
            m_codec.send((*it).get(), message);
        }
    }
    typedef std::set<muduo::net::TcpConnectionPtr> ConnectionList;
    muduo::net::TcpServer m_server;
    LengthHeaderCodec m_codec;
    ConnectionList m_connections;
};
int main(int argc, char const *argv[])
{
    mylogd("server");
    if(argc > 1) {
        muduo::net::EventLoop loop;
        uint16_t port = (uint16_t)(std::atoi(argv[1]));
        muduo::net::InetAddress listenAddr(port);
        ChatServer server(&loop, listenAddr);
        server.start();
        loop.loop();
    } else {
        printf("usage: %s port\n", argv[0]);
    }
    return 0;
}
