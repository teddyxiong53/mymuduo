#include "muduo/net/TcpConnection.h"
#include "mylog.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/Channel.h"
#include "muduo/net/Socket.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "muduo/net/SocketOps.h"

namespace muduo
{
namespace net
{

TcpConnection::TcpConnection(
    EventLoop *loop,
    const std::string& name,
    int sockfd,
    const InetAddress& localAddr,
    const InetAddress& peerAddr
)
 : m_loop(loop),
   m_name(name),
   m_state(kConnecting),
   m_socket(new Socket(sockfd)),
   m_channel(new Channel(loop, sockfd)),
   m_localAddr(localAddr),
   m_peerAddr(peerAddr)
{
    mylogd("tcp connection construct");
    m_channel->setReadCallback(
        std::bind(&TcpConnection::handleRead, this)
    );
    m_channel->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this)
    );
    m_channel->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this)
    );
    m_channel->setErrorCallback(
        std::bind(&TcpConnection::handleError, this)
    );

}

TcpConnection::~TcpConnection()
{
    mylogd("tcp connection desctruct");
}

void TcpConnection::connectEstablished()
{
    m_loop->assertInLoopThread();
    setState(kConnected);
    m_channel->enableReading();
    m_connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    setState(kDisconnected);
    m_channel->disableAll();
    m_connectionCallback(shared_from_this());
}

void TcpConnection::handleRead()
{
    char buf[65536];
    ssize_t n = ::read(m_channel->fd(), buf, sizeof(buf));
    if(n > 0) {
        m_messageCallback(shared_from_this(), buf, n);
    } else if(n == 0) {
        handleClose();
    } else {
        handleError();
    }

}

void TcpConnection::handleWrite()
{

}

void TcpConnection::handleClose()
{
    m_channel->disableAll();
    m_closeCallback(shared_from_this());
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(m_channel->fd());
    myloge("socket error:%d", err);
}


} // namespace net

} // namespace muduo
