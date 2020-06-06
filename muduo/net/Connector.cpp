#include "muduo/net/Connector.h"
#include "mylog.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/SocketOps.h"
#include <errno.h>

namespace muduo
{
namespace net
{
const int Connector::kMaxRetryDelayMs;
Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
 : m_loop(loop),
   m_serverAddr(serverAddr),
   m_connect(false),
   m_state(kDisconnected),
   m_retryDelayMs(kInitRetryDelayMs)
{

}

Connector::~Connector()
{
    m_loop->cancel(m_timerId);
}

void Connector::start()
{
    m_connect = true;
    m_loop->runInLoop(
        std::bind(&Connector::startInLoop, this)
    );
}
void Connector::startInLoop()
{
    m_loop->assertInLoopThread();
    if(m_connect) {
        connect();
    } else {
        myloge("");
    }
}

void Connector::connect()
{
    int sockfd = sockets::createNonblockingOrDie(m_serverAddr.family());
    int ret = sockets::connect(sockfd, m_serverAddr.getSockAddr());
    int savedErrno = (ret ==0)? 0: errno;
    switch(savedErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;
    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        myloge("serious error");
        sockets::close(sockfd);
        break;
    default:
        myloge("unexpected error:%d", savedErrno);
        sockets::close(sockfd);
        break;
    }
}

void Connector::restart()
{
    m_loop->assertInLoopThread();
    setState(kDisconnected);
    m_retryDelayMs = kInitRetryDelayMs;
    m_connect = true;
    startInLoop();
}

void Connector::stop()
{
    m_connect = false;
    m_loop->cancel(m_timerId);
}

void Connector::connecting(int sockfd)
{
    setState(kConnecting);
    m_channel.reset(new Channel(m_loop, sockfd));
    m_channel->setWriteCallback(
        std::bind(&Connector::handleWrite, this)
    );
    m_channel->setErrorCallback(
        std::bind(&Connector::handleError, this)
    );
    m_channel->enableWriting();
}

int Connector::removeAndResetChannel()
{
    m_channel->disableAll();
    m_loop->removeChannel(m_channel.get());
    int sockfd = m_channel->fd();
    m_loop->queueInLoop(
        std::bind(&Connector::resetChannel, this)
    );
    return sockfd;
}

void Connector::resetChannel()
{
    m_channel.reset();
}

void Connector::handleWrite()
{
    if(m_state == kConnecting) {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if(err) {
            myloge("socket error");
            retry(sockfd);
        } else {
            setState(kConnected);
            if(m_connect) {
                m_newConnectionCallback(sockfd);
            } else {
                sockets::close(sockfd);
            }
        }
    } else {
        //mylogd("m_state:");
    }
}


void Connector::handleError()
{
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    char buf[1024] = {0};
    myloge("socket error:%s", strerror_r(err, buf, 1024));
    retry(sockfd);
}

void Connector::retry(int sockfd)
{
    sockets::close(sockfd);
    setState(kDisconnected);
    if(m_connect) {
        m_timerId = m_loop->runAfter(
            m_retryDelayMs/1000.0,
            std::bind(&Connector::startInLoop, this)
        );
        m_retryDelayMs = std::min(m_retryDelayMs*2, kMaxRetryDelayMs);
    } else {
        mylogd("do not connect");
    }
}
} // namespace net

} // namespace muduo
