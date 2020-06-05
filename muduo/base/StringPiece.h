#ifndef _MUDUO_STRING_PIECE_H_
#define _MUDUO_STRING_PIECE_H_

#include <string.h>
#include <iosfwd>

#include "muduo/base/Types.h"
#include <string>

namespace muduo
{
class StringArg
{
public:
    StringArg(const char *str): m_str(str) {

    }
    StringArg(const std::string& str)
     : m_str(str.c_str())
    {

    }
    const char *c_str() {
        return m_str;
    }
private:
    const char * m_str;
};


class StringPiece
{
private:
    const char *m_ptr;
    int m_length;
public:
    //5个构造函数。
    StringPiece()
     : m_ptr(NULL), m_length(0)
    {

    }
    StringPiece(const char *str)
    : m_ptr(str),
      m_length(static_cast<int>(strlen(str)))
    {

    }
    StringPiece(const unsigned char *str)
    : m_ptr(reinterpret_cast<const char *>(str)),
      m_length((int)(strlen(m_ptr)))
    {

    }
    StringPiece(const std::string &str)
    : m_ptr(str.data()),
      m_length((int)(str.size()))
    {

    }
    StringPiece(const char *offset, int len)
     : m_ptr(offset), m_length(len)
    {

    }

    //查询类接口
    const char *data() {
        return m_ptr;
    }
    int size() {
        return m_length;
    }
    bool empty() {
        return m_length == 0;
    }
    const char *begin() {
        return m_ptr;
    }
    const char *end() {
        return m_ptr + m_length;
    }

    //修改类接口
    void clear() {
        m_ptr = NULL;
        m_length = 0;
    }
    void set(const char *buffer, int len) {
        m_ptr = buffer;
        m_length = len;
    }
    void set(const char *str) {
        m_ptr = str;
        m_length = static_cast<int>(strlen(str));
    }
    void set(const void *buffer, int len) {
        m_ptr = reinterpret_cast<const char *>(buffer);
        m_length = len;
    }

    char operator[](int i) {
        return m_ptr[i];
    }

    void remove_prefix(int n) {
        m_ptr += n;
        m_length -= n;
    }
    void remove_suffix(int n) {
        m_length -= n;
    }
    bool operator==(const StringPiece& x) {
        return (
            (m_length == x.m_length) &&
            (memcmp(m_ptr, x.m_ptr, m_length) == 0)
        );
    }
    bool operator!=(const StringPiece& x) {
        return !(*this == x);
    }

    std::string as_string() {
        return std::string(data(), size());
    }
    void CopyToString(std::string *target) {
        target->assign(m_ptr, m_length);
    }
    
};

} // namespace muduo


#endif //_MUDUO_STRING_PIECE_H_
