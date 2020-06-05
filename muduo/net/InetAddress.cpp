#include "muduo/net/InetAddress.h"
#include "mylog.h"
#include "muduo/net/SocketOps.h"

namespace muduo
{
namespace net
{

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    in_addr_t ip = INADDR_ANY;
    m_addr.sin_addr.s_addr = ::htonl(ip);
    m_addr.sin_port = htons(port);
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6)
{
    memset(&m_addr, 0, sizeof(m_addr));
    muduo::net::sockets::fromIpPort(ip.c_str(), port, &m_addr);
}

const struct sockaddr * InetAddress::getSockAddr() const {
    return sockets::sockaddr_cast(&m_addr);
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = "";
    sockets::toIpPort(buf, sizeof(buf), getSockAddr());
    return buf;
}

std::string InetAddress::toIp() const 
{
    char buf[64] = "";
    sockets::toIp(buf, sizeof(buf), getSockAddr());
    return buf;
}

} // namespace net

} // namespace muduo


