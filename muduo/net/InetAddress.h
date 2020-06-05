#ifndef _MUDUO_INET_ADDRESS_H_
#define _MUDUO_INET_ADDRESS_H_

#include "muduo/base/copyable.h"
#include "muduo/base/StringPiece.h"

#include <netinet/in.h>

namespace muduo
{
namespace net
{

class InetAddress: public muduo::copyable
{
public:
    explicit InetAddress(uint16_t port=0, bool loopbackOnly = false, bool ipv6 =false);
    InetAddress(StringArg ip, uint16_t port, bool ipv6=false);
    explicit InetAddress(const struct sockaddr_in &addr) {
        m_addr = addr;
    }
    sa_family_t family() const {
        return m_addr.sin_family;
    }
    std::string toIp() const ;
    std::string toIpPort() const;
    uint16_t toPort() const;
    const struct sockaddr * getSockAddr() const;
    void setSockAddr(const struct sockaddr_in& addr) {
        m_addr = addr;
    }
private:
    struct sockaddr_in m_addr;
};

} // namespace net

} // namespace muduo

#endif //_MUDUO_INET_ADDRESS_H_
