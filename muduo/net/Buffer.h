#ifndef _MUDUO_BUFFER_H_
#define _MUDUO_BUFFER_H_

#include "muduo/base/copyable.h"
#include <vector>
#include <algorithm>
#include <string>
#include <assert.h>
//#include "mylog.h"


namespace muduo
{
namespace net
{

class Buffer: muduo::copyable
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    Buffer()
     : m_buffer(kCheapPrepend+kInitialSize),
       m_readIndex(kCheapPrepend),
       m_writeIndex(kCheapPrepend)
    {

    }
    void swap(Buffer& rhs) {
        m_buffer.swap(rhs.m_buffer);
        std::swap(m_readIndex, rhs.m_readIndex);
        std::swap(m_writeIndex, rhs.m_writeIndex);
    }
    size_t readableBytes() const {
        //mylogd("w:%d, r:%d", m_writeIndex, m_readIndex);
        return m_writeIndex - m_readIndex;
    }
    size_t writableBytes() const {
        return m_buffer.size() - m_writeIndex;
    }
    size_t prependableBytes() const {
        return m_readIndex;
    }

    const char *peek() const {
        return begin() + m_readIndex;
    }
    void retrieve(size_t len) {
        m_readIndex += len;
    }
    void retrieveUntil(const char *end) {
        retrieve(end - peek());
    }
    void retrieveAll() {
        m_readIndex = kCheapPrepend;
        m_writeIndex = kCheapPrepend;
    }
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }
    std::string retrieveAsString(size_t len) {
        std::string str(peek(), len);
        retrieve(len);
        return str;
    }
    void append(const std::string& str) {
        append(str.data(), str.length());
    }
    void append(const char *data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }

    void append(const void *data, size_t len) {
        append(static_cast<const char *>(data), len);
    }
    void ensureWritableBytes(size_t len) {
        if(writableBytes() < len) {
            makeSpace(len);
        }
    }
    char *beginWrite() {
        return begin() + m_writeIndex;
    }
    const char *beginWrite() const {
        return begin() + m_writeIndex;
    }
    void hasWritten(size_t len) {
        m_writeIndex += len;
    }

    void prepend(const void *data, size_t len) {
        m_readIndex -= len;
        const char *d = (const char *)data;
        std::copy(d, d+len, begin() + m_readIndex);
    }

    void shrink(size_t reserve) {
        std::vector<char> buf(kCheapPrepend + readableBytes() + reserve);
        std::copy(
            peek(),
            peek() + readableBytes(),
            buf.begin() + kCheapPrepend
        );
        buf.swap(m_buffer);
    }
    ssize_t readFd(int fd, int* savedErrno);

private:
    char *begin() {
        return &*m_buffer.begin();
    }
    const char *begin() const {
        return &*m_buffer.begin();
    }

    void makeSpace(size_t len) {
        if(writableBytes() + prependableBytes() < len + kCheapPrepend) {
            m_buffer.resize(m_writeIndex+len);
        } else {
            size_t readable = readableBytes();
            std::copy(
                begin() + m_readIndex,
                begin() + m_writeIndex,
                begin() + kCheapPrepend
            );
            m_readIndex = kCheapPrepend;
            m_writeIndex = m_readIndex + readable;

        }
    }
    std::vector<char> m_buffer;
    size_t m_readIndex;
    size_t m_writeIndex;
};
} // namespace net

} // namespace muduo


#endif //_MUDUO_BUFFER_H_
