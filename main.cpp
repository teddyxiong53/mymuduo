
#include <stdio.h>
#include "muduo/base/Timestamp.h"
#include <iostream>
#include "muduo/base/CountDownLatch.h"
#include <thread>
#include <unistd.h>
#include "muduo/base/Thread.h"


using namespace muduo;

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
int main(int argc, char const *argv[])
{
    printf("muduo test\n");
    //test_Timestamp();
    //test_CountDownLatch();
    test_Thread();
    return 0;
}
