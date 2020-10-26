
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

# S11

这一步主要是加入Connector类。

这个对基础类提出了cancel定时器的需求。所以修改类都要加上cancel。

TimerId：加入了一个sequence序号。

Timer.h：加入sequence。和s_numCreated这个表示static变量。

TimerQueue：增加cancel。这才增加了ActiveTimerSet。

# S12

这一步是增加了TcpClient这个类。

实现一个反向的echo。服务端用nc -l 127.0.0.1 2001。

测试程序连接上来。服务端主动给客户端发消息，客户端 把消息原样返回。



到这里，就基本完成了。

接下来，写几个例子。看看各个组件是否工作正常。

实现这些例子。

```
chat
hub
```



# chat

3个文件：

codec.h

client.cpp

server.cpp

client，需要使用EventLoopThread，因为主线程需要进行输入。

作为ChatClient。应该有哪些接口？
连接服务端
断开服务器
发送消息
ChatClient需要实现的回调：
连接回调
消息回调
ChatClient需要有哪些成员？
一个TCPClient。
一个codec。编码和解码数据。
编码和解码，都是处理最前面4个字节的长度。

TcpClient帮ChatClient完成了哪些操作？
接收

TcpClient应该有哪些接口？
连接
断开
stop
	这个跟断开的区别是什么？
	是调用的Connector的stop接口。
	是提交一个任务到loop里执行。
		把对应的socket关闭掉。
	断开是操作一个connection。
	stop是操作Connector。
	
一个TCPClient，只有一个TcpConnection。

设置回调，使用std::move.

# hub



# snapcast

基于muduo来实现snapcast的功能。

本来是基于boost.asio的。

只考虑最简单的情况，pcm方式，不考虑其他编解码的。

也只使用c++11的语法。不引入boost的。

不考虑零配置的。

日志也只用mylog。

参数解析的也不用。

先写snapclient。这个是一个tcp client。

先写基本通信。

通路可以通之后，就开始写消息部分。这部分跟网络通信没有直接关系。

在common目录下新建message目录。

这个下面的层次关系

```
message.h
	这个定义BaseMessage。
	从这个开始写。
json_message
	直接继承BaseMessage。
	其他都是继承了JsonMessage。
```

然后写hello消息。

先就写这几个。其他的后面再添加。

ostream转普通string。

```
void someFunc(std::ostream out)
{
    std::stringstream ss;
    ss << out.rdbuf();
    std::string myString = ss.str();
}
```

ostream的构造方法，只有一种，需要一个streambuf指针。

现在我要传递ostringstream来做序列化。

用stringstream是不行的。

printf打印不出来。cout可以打印出来。

前面有字符不对。

我觉得可能是我的代码有问题。

在原始代码上做这个测试看看。

也还是一样。那可能就是ostringstream用得不对。

知道了。序列化的时候，并不是都写的字符串。前面的部分，是写的二进制。所以当字符串打印，当然是不对的。

现在用nc起监听，可以正常连接，也可以收到Hello消息。

然后是发送Time消息。

这个的目的是什么？

获取跟服务端的时间差？

先不做吧。

controller是需要一个线程的，用来定时往服务器发送消息。

Time消息也是直接继承自BaseMessage，而不是JsonMessage。

定时发Time消息写好了。

然后是处理消息接收。这个是主要的。

开始加player和stream的。

stream依赖DoubleBuffer。

WireChunk消息直接继承BaseMessage。

还是需要把exception还是要加进来。

不用的话，都用return来改造太麻烦。



有些消息，是需要发送马上等待回复的同步方式，这种muduo好像没有。

只能靠上层自己依靠变量等方式来模拟同步方式了。

snapcast里，也是靠一个condition_variable来做的。



现在笔记本上运行snapserver。

台式机上运行我自己写的snapclient。连接会失败。

我把snapclient的retry关闭掉。

不然会反复重连，导致打印打印，不利于分析。

现在getsocketopt得到的错误码是32 。

为什么会有这个错误？

这个错误是broke pipe。

是因为对端关闭了socket导致的。

我用nc来监听1704端口。连接没有问题。

那么应该就是snapserver发现我写的snapclient不合法，断开了对应的连接。

snapserver这边日志：

````
2020-10-26 10-13-20.824 [Notice] (handleAccept) StreamServer::NewConnection: 172.16.2.83
2020-10-26 10-13-20.832 [Debug] (operator()) getNextMessage: 23786, size: 176, id: 0, refers: 0
2020-10-26 10-13-20.832 [Err] (operator()) unknown message type received: 23786, size: 176
2020-10-26 10-13-20.832 [Info] (onDisconnect) onDisconnect: 
````

是收到了未知类型的消息。

消息类型是23786。这个明显不对。

消息类型都是基本枚举，数字很小的。

那应该就是我的消息序列化有问题。

怎么判断是否正确呢？

在序列化之前就不对。发送和接收解析是对得上的。

知道原因了。相当于type没有被初始化。

```
    BaseMessage(message_type type)
    {
        mylogd("type:%d", type);
        this->type = type;//我开始这里是写的type = type，相当于空操作。type没有被赋值。
        id = 0;
        refersTo = 0;
    }
```

现在消息可以发送到服务端这边了。但是估计消息有问题，直接把snapserver这边弄挂了。



