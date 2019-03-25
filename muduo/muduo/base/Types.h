#ifndef MUDUO_BASE_TYPE_H
#define MUDUO_BASE_TYPE_H

#include <stdint.h>
#include <string.h>//memset
#include <string>

namespace mudo
{
using std::string;
inline void memZero(void *p, size_t n)
{
    memset(p, 0, n);
}


}//muduo

#endif //MUDUO_BASE_TYPE_H