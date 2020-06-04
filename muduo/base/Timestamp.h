#ifndef _MUDUO_TIMESTAMP_H_
#define _MUDUO_TIMESTAMP_H_

#include <stdint.h>
#include "muduo/base/copyable.h"
#include <utility>
#include <string>
#include <time.h>
namespace muduo
{



class Timestamp
{
public:
    Timestamp(): m_usValue(0) {}
    explicit Timestamp(int64_t value) : m_usValue(value) {}

    void swap(Timestamp& that) {
        std::swap(m_usValue, that.m_usValue);
    }
    std::string toString() const;
    std::string toFormattedString(bool showUs=true) const;

    static Timestamp now();
    static Timestamp invalid() {
        return Timestamp();
    }
    static Timestamp fromUnixTime(time_t t) {
        fromUnixTime(t, 0);
    }
    static Timestamp fromUnixTime(time_t t, int us) {
        return Timestamp(t*kUsPerSecond + us);
    }
    static const int kUsPerSecond = 1000*1000;
private:
    int64_t m_usValue;//从1970年开始的ms值。30万年之后会溢出。
};

} // namespace muduo


#endif
