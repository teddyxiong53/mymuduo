#include "muduo/net/Buffer.h"
#include "mylog.h"
#include "SocketOps.h"
#include <errno.h>
#include <sys/uio.h>

namespace muduo
{
namespace net
{

ssize_t Buffer::readFd(int fd, int *savedErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writeable = writableBytes();
    vec[0].iov_base = begin() + m_writeIndex;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    const ssize_t n = sockets::readv(fd, vec, 2);
    //mylogd("n:%d", n);
    if(n < 0) {
        *savedErrno = errno;
    } else if(n <= writeable) {
        m_writeIndex += n;
    } else {
        m_writeIndex = m_buffer.size();
        append(extrabuf, n-writeable);

    }
    return n;
}
} // namespace net

} // namespace muduo

