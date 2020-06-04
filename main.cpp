
#include <stdio.h>
#include "muduo/base/Timestamp.h"
#include <iostream>


void test_Timestamp()
{
    std::cout << "time sec :" << Timestamp::now().toString() << "\n";
    std::cout << "Time format string:" << Timestamp::now().toFormattedString() << "\n";
}
int main(int argc, char const *argv[])
{
    printf("muduo test\n");
    test_Timestamp();
    return 0;
}
