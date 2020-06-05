#ifndef _MUDUO_TYPES_H_
#define _MUDUO_TYPES_H_

namespace muduo
{

template <typename To, typename From>
inline To implicit_cast(From const &f)
{
    return f;
}

template <typename To, typename From>
inline To down_cast(From *f)
{
    return static_cast<To>(f);
}
} // namespace muduo


#endif //_MUDUO_TYPES_H_
