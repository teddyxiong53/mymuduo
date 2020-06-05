#ifndef _MUDUO_CALLBACKS_H_
#define _MUDUO_CALLBACKS_H_

#include <functional>
#include <memory>

namespace muduo
{
namespace net
{

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

typedef std::function<void()> TimerCallback;

class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&,
        const char *data,
        ssize_t len
    )> MessageCallback;

typedef std::function<void (const TcpConnectionPtr& )> CloseCallback;

} // namespace net

} // namespace muduo


#endif //_MUDUO_CALLBACKS_H_
