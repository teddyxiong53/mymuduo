#include "muduo/net/Acceptor.h"
#include "mylog.h"
#include "muduo/net/Socket.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/SocketOps.h"
#include "muduo/net/InetAddress.h"

namespace muduo
{
namespace net
{
Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
 : m_loop(loop),
   m_acceptSocket(sockets::createNonblockingOrDie(listenAddr.family())),
   m_acceptChannel(loop, m_acceptSocket.fd()),
   m_listening(false)
{
    m_acceptSocket.setReuseAddr(true);
    m_acceptSocket.bindAddress(listenAddr);
    m_acceptChannel.setReadCallback(
        std::bind(&Acceptor::handleRead, this)
    );
}

void Acceptor::listen()
{
    m_loop->assertInLoopThread();
    m_listening = true;
    m_acceptSocket.listen();
    m_acceptChannel.enableReading();
}

void Acceptor::handleRead()
{
    m_loop->assertInLoopThread();
    InetAddress peerAddr(0);
    int connfd = m_acceptSocket.accept(&peerAddr);
    if(connfd > 0) {
        if(m_newConnectionCallback) {
            m_newConnectionCallback(connfd, peerAddr);
        } else {
            sockets::close(connfd);
        }
    }
}
} // namespace net

} // namespace muduo


