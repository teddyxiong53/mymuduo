#pragma once

#include "muduo/base/noncopyable.h"
#include "muduo/net/TcpClient.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/TcpConnection.h"
#include "common/message/message.h"

using muduo::net::TcpClient;
using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::net::TcpConnectionPtr;
using muduo::net::Buffer;

class Controller : public muduo::noncopyable
{
public:
    Controller(EventLoop* loop, const InetAddress& addr);
    void connect();
    void disconnect();
    void write(msg::BaseMessage* message);
    void onConnection(const TcpConnectionPtr & conn) ;
    void onMessage(const TcpConnectionPtr& conn,
        Buffer* buf,
        muduo::Timestamp receiveTime);
private:
    TcpClient m_client;
    std::mutex m_mutex;
    TcpConnectionPtr m_connection;
    std::string m_hostId;
    size_t m_instance;
};
