#ifndef _MUDUO_SOCKET_OPS_H_
#define _MUDUO_SOCKET_OPS_H_

#include <arpa/inet.h>
#include <endian.h>

namespace muduo
{
namespace net
{
namespace sockets
{


struct sockaddr *sockaddr_cast( struct sockaddr_in *addr);

const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);


const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr);


int createNonblockingOrDie(sa_family_t family);
void bindOrDie(int sockfd, const struct sockaddr *addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in *addr);
int connect(int sockfd, const struct sockaddr* addr);
ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec * iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t count);
void close(int sockfd);
void shutdownWrite(int sockfd);
void toIpPort(char *buf, size_t size, const struct sockaddr *addr);
void toIp(char *buf, size_t size, const struct sockaddr *addr);
void fromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr);
int getSocketError(int sockfd);
struct sockaddr_in getLocalAddr(int sockfd);
struct sockaddr_in getPeerAddr(int sockfd);
} // namespace sockets

} // namespace net

} // namespace muduo


#endif //_MUDUO_SOCKET_OPS_H_
