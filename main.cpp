
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
int main(int argc, char const *argv[])
{
    printf("------------muduo test begin --------------\n");
    //test_Timestamp();
    //test_CountDownLatch();
    //test_Thread();
    //test_EventLoopS00();
    test_EventLoopS01();
    printf("------------muduo test end --------------\n");
    return 0;
}
