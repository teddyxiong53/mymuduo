
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

# S07

这一步主要是引入Buffer机制。

之前的MessageCallback是：

```
typedef boost::function<void (const TcpConnectionPtr&,
                              const char* data,
                              ssize_t len)> MessageCallback;
```

现在改成：

```
typedef boost::function<void (const TcpConnectionPtr&,
                              Buffer* buf,
                              Timestamp)> MessageCallback;
```

时间戳不是必须的，只是一个辅助性的东西。

写完后，调试出现了死机问题。

是因为我打印时写错了。尝试对一个int值进行%s打印。当然就是非法内存访问了。

```
mylogd("%s", int_val); 
```

# S08

这一步，实现服务端的发送操作。

采用电平触发方式。所以不要一直关注write，只在需要发送的时候，进行关注，发送完成，马上取消关注。

否则会造成busy-loop。

没有新增类。只是修改。

Channel：增加3个方法，enableWriting、disableWriting、isWriting。

Socket：增加一个shutdownWrite方法。

TcpCopnnection：

增加send和shutdown函数。及对应的InLoop函数。

增加m_outputBuffer成员变量。

枚举增加kDisconnecting。这个是在shutdown里设置，在InLoop函数里才变成kDisconnected。

实现之前留空的handleWrite函数。

现在实现了发送，就可以开始做echo了。

# S09

这一步主要是完善。实现写完成事件和HighWaterMark。

Callbacks.h：增加WriteCompleteCallback回调类型。

EventLoop.cpp：靠全局静态类的方式来设置SIGPIPE。

Socket.h：增加setTcpNoDelay。

TcpConnection.h：增加setTcpNoDelay、setWriteCompleteCallback、

TcpServer.h：增加setWriteCompleteCallback。



# S10

这一步主要是增加EventLoopThreadPool。

除了增加将EventLoopThreadPool，只修改TcpServer。

