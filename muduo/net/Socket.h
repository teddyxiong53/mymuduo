#ifndef _MUDUO_SOCKET_H_
#define _MUDUO_SOCKET_H_
#include "muduo/base/noncopyable.h"

namespace muduo
{
namespace net
{
class InetAddress;
class Socket: muduo::noncopyable
{
public:
    explicit Socket(int sockfd)
     : m_sockfd(sockfd)
    {

    }
    ~Socket();
    int fd() {
        return m_sockfd;
    }
    void bindAddress(const InetAddress &localAddr);
    void listen();
    int accept(InetAddress* peerAddr);
    void setReuseAddr(bool on);
    void shutdownWrite();
    
private:
    int m_sockfd;
};

} // namespace net

} // namespace muduo


#endif //_MUDUO_SOCKET_H_
