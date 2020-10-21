#ifndef _MUDUO_CALLBACKS_H_
#define _MUDUO_CALLBACKS_H_

#include <functional>
#include <memory>
#include "muduo/base/Timestamp.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

namespace muduo
{
namespace net
{



typedef std::function<void()> TimerCallback;

class TcpConnection;
class Buffer;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&,
        Buffer* buf,
        muduo::Timestamp
    )> MessageCallback;

typedef std::function<void (const TcpConnectionPtr& )> CloseCallback;

typedef std::function<void (const TcpConnectionPtr& conn)> WriteCompleteCallback;

} // namespace net

} // namespace muduo


#endif //_MUDUO_CALLBACKS_H_
