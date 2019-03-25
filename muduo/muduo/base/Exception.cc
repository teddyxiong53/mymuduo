#include <muduo/base/Exception.h>
#include <muduo/base/CurrentThread.h>

namespace muduo
{
Exception::Exception(std::string msg)
 : message_(std::move(msg)),
   stack_(CurrentThread::stackTrace(false))
{

}
} // muduo
