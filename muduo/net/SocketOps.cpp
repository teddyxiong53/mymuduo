#include "muduo/net/SocketOps.h"
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include "mylog.h"
#include "muduo/base/Types.h"
#include <inttypes.h>
#include <string.h>

namespace muduo
{
namespace net
{
namespace sockets
{



const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr)
{
    return static_cast<const struct sockaddr*>(implicit_cast<const void *>(addr));
}

 struct sockaddr *sockaddr_cast( struct sockaddr_in *addr)
{
    return static_cast< struct sockaddr*>(implicit_cast< void *>(addr));
}

const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr)
{
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void *>(addr));
}



int createNonblockingOrDie(sa_family_t family)
{
    int fd = ::socket(family, SOCK_STREAM|SOCK_NONBLOCK| SOCK_CLOEXEC, IPPROTO_TCP);
    if(fd < 0) {
        myloge("create socket fail");
        abort();
    }
    return fd;
}

void bindOrDie(int sockfd, const struct sockaddr *addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    if(ret < 0) {
        myloge("bind fail");
        abort();
    }
}

void listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if(ret < 0) {
        myloge("listen fail");
        abort();
    }
}

int accept(int sockfd, struct sockaddr_in *addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0) {
        int savedErrno = errno;
        mylogd("accept4 fail");
        switch(savedErrno) {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO:
        case EPERM:
        case EMFILE:
            errno = savedErrno;
            break;//上面都是非严重错误
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
        default:
            myloge("serious error:%d", savedErrno);
            abort();
        }
    }
    return connfd;
}


int connect(int sockfd, const struct sockaddr* addr)
{
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

ssize_t read(int sockfd, void *buf, size_t count)
{
    return ::read(sockfd, buf, count);
}

ssize_t readv(int sockfd, const struct iovec * iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t write(int sockfd, const void *buf, size_t count)
{
    return ::write(sockfd, buf, count);
}

void close(int sockfd)
{
    ::close(sockfd);
}

void shutdownWrite(int sockfd)
{
    if(::shutdown(sockfd, SHUT_WR) < 0) {
        myloge("shutdown write fail");
    }
}

void toIpPort(char *buf, size_t size, const struct sockaddr *addr)
{
    toIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
    uint16_t port = ::ntohs(addr4->sin_port);
    snprintf(buf+end, size-end, ":%u", port);
}
void toIp(char *buf, size_t size, const struct sockaddr *addr)
{
    if(addr->sa_family == AF_INET) {
        const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
}

void fromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = ::htons(port);
    int ret = ::inet_pton(AF_INET, ip, &addr->sin_addr);
    if(ret < 0) {
        myloge("inet_pton fail");
    }
}


int getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    int ret;
    ret = ::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen);
    if(ret < 0) {
        return errno;
    } else {
        return optval;
    }
}

struct sockaddr_in getLocalAddr(int sockfd)
{
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localAddr));
    int ret;
    ret = ::getsockname(sockfd, sockaddr_cast(&localAddr), &addrlen);
    if(ret < 0) {
        myloge("getsockname fail");
    }
    return localAddr;
}
struct sockaddr_in getPeerAddr(int sockfd)
{
    struct sockaddr_in peerAddr;
    memset(&peerAddr, 0, sizeof(peerAddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peerAddr));
    int ret;
    ret = ::getpeername(sockfd, sockaddr_cast(&peerAddr), &addrlen);
    if(ret < 0) {
        myloge("getpeername fail");
    }
    return peerAddr;
}


} // namespace sockets

} // namespace net

} // namespace muduo

