
#include <stdio.h>
#include "muduo/base/Timestamp.h"
#include <iostream>
#include "muduo/base/CountDownLatch.h"
#include <thread>
#include <unistd.h>
#include "muduo/base/Thread.h"
#include "muduo/net/EventLoop.h"
#include "mylog.h"
#include <sys/timerfd.h>
#include "muduo/net/Channel.h"
#include <string.h>
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/Acceptor.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/SocketOps.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/TcpConnection.h"

using muduo::Timestamp;
using muduo::CountDownLatch;
using muduo::Thread;

void test_Timestamp()
{
    std::cout << "time sec :" << Timestamp::now().toString() << "\n";
    std::cout << "Time format string:" << Timestamp::now().toFormattedString() << "\n";
}
void CountDownLatchThread(CountDownLatch *c)
{
    int count = 3;
    while(count--) {
        sleep(1);
        printf("waiting ...\n");
        c->countDown();
    }
    printf("end of thread\n");
}
void test_CountDownLatch()
{
    CountDownLatch *c = new CountDownLatch(3);
    std::thread thr(CountDownLatchThread, c);
    c->wait();
    thr.join();
    printf("wait return\n");
}

void threadFunc()
{
    sleep(2);
    printf("thread func\n");
}
void test_Thread()
{
    Thread *t = new Thread(threadFunc, "test-muduo-thread");
    t->start();
    t->join();
    printf("test thread end\n");
}
void test_EventLoopS00()
{
    muduo::net::EventLoop loop;
    loop.loop();
}
muduo::net::EventLoop *g_loop;

void timeout()
{
    mylogd("timeout happen");
    g_loop->quit();
}
void test_EventLoopS01()
{
    muduo::net::EventLoop loop;
    g_loop = &loop;
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    muduo::net::Channel channel(&loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();

    struct ::itimerspec howlong;
    memset(&howlong, 0, sizeof(howlong));
    howlong.it_value.tv_sec = 3;
    timerfd_settime(timerfd, 0, &howlong, NULL);
    loop.loop();
    close(timerfd);

}
void print()
{
    static int i = 0;
    mylogd("i:%d", ++i);
}
void test_EventLoopS02()
{
    muduo::net::EventLoop loop;
    g_loop = &loop;
    loop.runAfter(1, print);
    loop.runAfter(2, print);
    loop.loop();
}

void run3()
{
    mylogd("");
    g_loop->quit();
}
void run2()
{
    mylogd("");
    g_loop->queueInLoop(run3);
}
void run1()
{
    mylogd("");
    g_loop->runInLoop(run2);
}
void test_EventLoopS03()
{
    muduo::net::EventLoop loop;
    g_loop = &loop;
    loop.runAfter(2, run1);
    loop.loop();
}
void runInThread()
{
    mylogd("pid:%d, tid:%d", getpid(), muduo::CurrentThread::tid());
}
void test_EventLoopS03_2()
{
    mylogd("pid:%d, tid:%d", getpid(), muduo::CurrentThread::tid());
    muduo::net::EventLoopThread loopThread;
    muduo::net::EventLoop *loop = loopThread.startLoop();
    loop->runInLoop(runInThread);
    sleep(1);
    loop->runAfter(2, runInThread);
    sleep(3);
    loop->quit();

}

void newConnection(int sockfd, const muduo::net::InetAddress& addr)
{
    mylogd("accept from %s", addr.toIpPort().c_str());
    ::write(sockfd, "how are you?\n", 13);
    muduo::net::sockets::close(sockfd);
}
void test_EventLoopS04()
{
    muduo::net::InetAddress listenAddr(2001);
    muduo::net::EventLoop loop;

    muduo::net::Acceptor acceptor(&loop, listenAddr);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();
    loop.loop();
}

void onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(conn->connected()) {
        mylogd("connected");
    } else {
        mylogd("disconnnected");
    }
}
void onMessage(const muduo::net::TcpConnectionPtr& conn,
    const char *data, ssize_t len)
{
    mylogd("get message:%s", data);
}

void test_EventLoopS05()
{
    muduo::net::InetAddress listenAddr(2001);
    muduo::net::EventLoop loop;

    muduo::net::TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();
    loop.loop();
}
void test_EventLoopS06()
{
    test_EventLoopS05();
}
int main(int argc, char const *argv[])
{
    printf("------------muduo test begin --------------\n");
    //test_Timestamp();
    //test_CountDownLatch();
    //test_Thread();
    //test_EventLoopS00();
    //test_EventLoopS01();
    //test_EventLoopS02();
    //test_EventLoopS03();
    //test_EventLoopS03_2();
    // test_EventLoopS04();
    //test_EventLoopS05();
    test_EventLoopS06();
    printf("------------muduo test end --------------\n");
    return 0;
}
