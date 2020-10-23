#pragma once

#include "muduo/base/noncopyable.h"
#include "muduo/net/TcpClient.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/TcpConnection.h"
#include "common/message/message.h"
#include "muduo/base/Thread.h"
#include "common/message/server_settings.h"
#include "client/stream.h"
#include "client/player/player.h"
#include "client/decoder/decoder.h"

using muduo::net::TcpClient;
using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::net::TcpConnectionPtr;
using muduo::net::Buffer;
// using muduo::Thread;

class Controller : public muduo::noncopyable
{
public:
    Controller(EventLoop* loop, const InetAddress& addr);
    void connect();
    void disconnect();
    void sendMessage(msg::BaseMessage* message);
    void onConnection(const TcpConnectionPtr & conn) ;
    void onMessage(const TcpConnectionPtr& conn,
        Buffer* buf,
        muduo::Timestamp receiveTime);
    void start();
    void stop();
    void worker();
    bool sendTimeSyncMessage(long after);
private:
    TcpClient m_client;
    std::mutex m_mutex;
    TcpConnectionPtr m_connection;
    std::string m_hostId;
    size_t m_instance;
    muduo::Thread m_controllerThread;
    bool m_active;
    std::shared_ptr<msg::ServerSettings> m_serverSettings;
    std::shared_ptr<Stream> m_stream;
    std::unique_ptr<decoder::Decoder> m_decoder;
    std::unique_ptr<Player> m_player;
    std::shared_ptr<msg::CodecHeader> m_headerChunk;
    SampleFormat m_sampleFormat;
    int m_latency;
};
