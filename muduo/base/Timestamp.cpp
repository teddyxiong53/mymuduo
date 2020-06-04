#include "Timestamp.h"
#include <sys/time.h>

std::string Timestamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = m_usValue/kUsPerSecond;
    int64_t us = m_usValue%kUsPerSecond;
    snprintf(buf, sizeof(buf)-1, "%ld.%ld", seconds, us);
    return buf;
}
std::string Timestamp::toFormattedString(bool showUs) const
{
    struct tm tm;
    time_t sec = m_usValue/kUsPerSecond;
    gmtime_r(&sec, &tm);
    char buf[64] = {0};
    if(showUs) {
        snprintf(buf, sizeof(buf)-1, "%04d%02d%02d %02d:%02d:%02d.%06d",
            tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, static_cast<int>(m_usValue%kUsPerSecond));
    } else {
        snprintf(buf, sizeof(buf)-1, "%04d%02d%02d %02d:%02d:%02d",
            tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    return buf;
}

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t sec = tv.tv_sec;
    return Timestamp(sec*kUsPerSecond + tv.tv_usec);
}

