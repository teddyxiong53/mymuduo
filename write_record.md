
先实现Timestamp.h。
把依赖的进行实现。
这个的依赖相对较少。

mutex是否有必要自己实现呢？
我觉得没有必要。
直接用c++标准的mutex。

thread有没有必要自己实现？
这个还是有必要，因为标准的thread的id，还是指针值。
这个确实不太好。

atomic的，这个也用c++标准的。

```
‘gettid’ was not declared in this scope
```
这样解决：
```
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
```

现在线程可以工作。

接下来就是实现EventLoop和定时器的。

既然写EventLoop，那么就可以参考muduo书籍的第八章做。



# S00

这个就一个EventLoop类。

# S01

这个加入Channel类和Poller类。

# S02

这个开始计入TimerQueue到EventLoop里。依赖timerfd。

TimerQueue就有3个类：Timer、TimerId、TimerQueue。

先实现Timer。

# S03

这个的主要是加入eventfd来支持在io线程里执行回调。

然后加入EventLoopThread。

主要改动在EventLoop里。然后用这个机制改造TimerQueue，让它变得线程安全。

# S04

这个就是开始加入tcp相关的内容了。

这里开始变得困难一些了。

这一步，还是只加入了一个Acceptor类。不过配套有SocketOps、Socket、InetAddress这3个文件。

先写SocketOps这个。

这个没有类，都是封装在muduo::net::sockets这个namespace里的一些函数。



InetAddress 这个会引入StringPiece类。

# S05

这一步开始加入TCPServer和TcpConnection这2个类。

# S06

这一步没有新增类。是对前面的类进行改动，用来支持连接断开。

Callbacks.h里增加一个CloseCallback函数类型。

Channel.h里增加析构函数，setCloseCallback，disableAll函数。

EventLoop.h里增加removeChannel函数。它只是转而调用Poller里的removeChannel函数。

Poller.h里增加removeChannel函数。

TcpConnection增加：setCloseCallback、connectDestroyed、handleClose、handleWrite、handleError、

增加了kDisconnected枚举。

TCPServer里增加：removeConnection。

这个没有新增测试用例。直接用上一个的测试用例就可以。然后客户端练上来，主动断开连接，可以看到相关的打印。





