#include "muduo/net/Socket.h"
#include "mylog.h"
#include "muduo/net/SocketOps.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include "muduo/net/InetAddress.h"

namespace muduo
{
namespace net
{
Socket::~Socket()
{
    sockets::close(m_sockfd);
}

void Socket::bindAddress(const InetAddress& addr)
{
    sockets::bindOrDie(m_sockfd, addr.getSockAddr());
}
void Socket::listen()
{
    sockets::listenOrDie(m_sockfd);
}
int Socket::accept(InetAddress* peerAddr)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    int connfd = sockets::accept(m_sockfd, &addr);
    if(connfd >= 0) {
        peerAddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::setReuseAddr(bool on)
{
    int optval = on? 1: 0;
    int ret = ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, (socklen_t)(sizeof(optval)));
    if(ret < 0) {
        myloge("set reuse addr fail");
    }
}

void Socket::shutdownWrite()
{
    sockets::shutdownWrite(m_sockfd);
}


void Socket::setTcpNoDelay(bool on)
{
    int optval = on? 1: 0;
    ::setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

} // namespace net

} // namespace muduo

