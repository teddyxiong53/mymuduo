#ifndef _MUDUO_NONCOPYABLE_H_
#define _MUDUO_NONCOPYABLE_H_

namespace muduo
{
class noncopyable
{
public:
    noncopyable(const noncopyable& ) = delete;
    void operator=(const noncopyable& ) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
} // namespace


#endif
