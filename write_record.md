
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

