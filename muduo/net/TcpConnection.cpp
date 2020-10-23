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
        std::bind(&TcpConnection::handleRead, this, _1)
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

int TcpConnection::getSocketFd()
{
    return m_socket->fd();
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

void TcpConnection::handleRead(Timestamp receiveTime)
{
    //ssize_t n = ::read(m_channel->fd(), buf, sizeof(buf));
    int savedErrno = 0;
    ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &savedErrno);
    if(n > 0) {
        //mylogd("readable bytes:%d, &m_inputBuffer;0x%p", m_inputBuffer.readableBytes(), &m_inputBuffer);

        m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
    } else if(n == 0) {
        handleClose();
    } else {
        handleError();
    }

}

void TcpConnection::handleWrite()
{
    m_loop->assertInLoopThread();
    if(m_channel->isWriting()) {
        ssize_t n = ::write(
            m_channel->fd(),
            m_outputBuffer.peek(),
            m_outputBuffer.readableBytes()
        );
        if(n > 0) {
            m_outputBuffer.retrieve(n);
            if(m_outputBuffer.readableBytes() == 0) {
                //说明buffer里的数据都发送完成了。
                //停止关注可写事件。
                m_channel->disableWriting();
                if(m_writeCompleteCallback) {
                    m_loop->queueInLoop(
                        std::bind(m_writeCompleteCallback, shared_from_this())
                    );
                }
                if(m_state == kDisconnecting) {
                    shutdownInLoop();
                }
            } else {
                //说明还有数据要发送。
                mylogd("going to write more data");
            }
        } else {
            myloge("write error");
        }
    } else {
        myloge("connection is down");
    }
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

void TcpConnection::send(const void *data, int len)
{
    send(StringPiece((const char *) data, len));
}
void TcpConnection::send(const StringPiece& message)
{
    if(m_state == kConnected) {
        if(m_loop->isInLoopThread()) {
            sendInLoop(message);
        } else {
            void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
            m_loop->runInLoop(
                std::bind(fp, this, message.as_string())
            );
        }
    }
}

void TcpConnection::send(Buffer* buf)
{
    if(m_state == kConnected) {
        if(m_loop->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        } else {
            void (TcpConnection::*fp)(const StringPiece &message) = &TcpConnection::sendInLoop;
            m_loop->runInLoop(
                std::bind(
                    fp,
                    this,
                    buf->retrieveAllAsString()
                )
            );
        }
    }
}
void TcpConnection::sendInLoop(const StringPiece& message)
{
    sendInLoop(message.data(), message.size());
}


void TcpConnection::sendInLoop(const void *data, size_t len)
{
    m_loop->assertInLoopThread();
    ssize_t nwrote = 0;
    if(m_state == kDisconnected) {
        myloge("disconnected ,give up writing");
        return;
    }

    if(!m_channel->isWriting() && m_outputBuffer.readableBytes()==0) {
        //如果当前输出buffer里没有内容，则直接写。
        //第一次发送，就是这种情况。
        nwrote = ::write(m_channel->fd(), data, len);
        if(nwrote >= 0) {
            if((size_t)nwrote < len) {
                //说明没有写完。
                mylogd("going to write more data");
            } else if(m_writeCompleteCallback) {
                //说明写完了
                m_loop->queueInLoop(
                    std::bind(m_writeCompleteCallback, shared_from_this())
                );
            }
        } else {
            //小于0
            //说明write出错了。
            if(errno != EWOULDBLOCK) {
                mylogd("write error ");
            }
        }
    }
    if((size_t)nwrote < len) {
        m_outputBuffer.append(data+nwrote, len-nwrote);
        if(!m_channel->isWriting()) {
            m_channel->enableWriting();
        }
    }
}


void TcpConnection::shutdown()
{
    if(m_state == kConnected) {
        setState(kDisconnecting);
        m_loop->runInLoop(std::bind(
            &TcpConnection::shutdownInLoop, this
        ));
    }
}

void TcpConnection::shutdownInLoop()
{
    m_loop->assertInLoopThread();
    if(!m_channel->isWriting()) {
        m_socket->shutdownWrite();
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    m_socket->setTcpNoDelay(on);
}

} // namespace net

} // namespace muduo
