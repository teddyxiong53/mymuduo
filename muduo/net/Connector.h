#ifndef _MUDUO_CONNECTOR_H_
#define _MUDUO_CONNECTOR_H_

#include "muduo/base/noncopyable.h"
#include <memory>
#include "muduo/net/InetAddress.h"
//include "muduo/net/Callbacks.h"
#include "muduo/net/TimerId.h"
#include <functional>
#include "muduo/net/Channel.h"

namespace muduo
{
namespace net
{

class EventLoop;
class Channel;

class Connector: muduo::noncopyable
{
public:
    typedef std::function<void (int sockfd)> NewConnectionCallback;
    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback &cb) {
        m_newConnectionCallback = cb;
    }
    void start();
    void stop();
    void restart();

    const InetAddress& serverAddress() const {
        return m_serverAddr;
    }

private:
    enum States {
        kDisconnected,
        kConnecting,
        kConnected,
    };
    static const int kMaxRetryDelayMs = 30000;
    static const int kInitRetryDelayMs = 500;

    void setState(States s) {
        m_state = s;
    }
    void startInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop *m_loop;
    bool m_connect;
    InetAddress m_serverAddr;
    States m_state;
    std::unique_ptr<Channel> m_channel;
    NewConnectionCallback m_newConnectionCallback;
    int m_retryDelayMs;
    TimerId m_timerId;
};
typedef std::shared_ptr<Connector> ConnectorPtr;

} // namespace net

} // namespace muduo

#endif
