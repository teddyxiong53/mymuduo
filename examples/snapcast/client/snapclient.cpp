#include "mylog.h"
#include "common/string_utils.h"
#include "client/player/pcm_device.h"
#include <memory>
#include "common/daemon.h"
#include <vector>
#include "client/player/alsa_player.h"
#include "muduo/base/noncopyable.h"
#include "controller.h"
#include "muduo/net/EventLoopThread.h"
#include <iostream>

//默认不用daemon
// #define USE_DAEMON
using namespace muduo;
// using namespace muduo::net;
using muduo::net::EventLoopThread;



PcmDevice getPcmDevice(const std::string& soundCard)
{
    std::vector<PcmDevice> pcmDevices = AlsaPlayer::pcm_list();
    for(auto dev: pcmDevices) {
        mylogd("name:%s", dev.name.c_str());
        if(dev.name.find(soundCard) != std::string::npos) {
            return dev;
        }
    }
    PcmDevice empty;
    return empty;
}

#if 0
int main(int argc, char const *argv[])
{
    #ifdef USER_DAEMON
    std::unique_ptr<Daemon> daemon;
    std::string pidFile = "/var/run/snapclient/pid";
    std::string user = "";
    std::string group = "";
    daemon.reset(new Daemon(user, group, pidFile));
    if(daemon->daemonize() == false) {
        //失败了
        exit(1);
    }
    mylogd("daemon start");
    #endif
    //获取alsa设备列表
    // PcmDevice pcmDevice = getPcmDevice("default");
    int instance = 1;
    std::string host = "127.0.0.1";
    int port = 2020;
    int latency = 0;
    EventLoopThread loopThread;
    InetAddress serverAddr(host, port);
    Controller controller(loopThread.startLoop(), serverAddr);
    controller.start();

    //
    while(1) {
        sleep(1);
    }

    controller.disconnect();
    return 0;
}
#else
#include "common/message/message.h"
#include "common/message/hello.h"

void test_message()
{
    msg::Hello msg("123", "123", 1);
    std::ostringstream oss;
    oss.str("");
    msg.serialize(oss);
    // mylogd("result:%s", oss.str().c_str());
    std::cout << oss.str() << "\n";
    oss.str("123");
    std::cout << oss.str() << "\n";
}
void test_vector()
{
    std::string s1("123");
    std::string s2("Any elements held in the container before the call are destroyed and replaced by newly constructed elements (no assignments of elements take place).");
    std::vector<char> buffer(s1.begin(),s1.end());
    buffer.resize(2);
    buffer.assign(s2.begin(), s2.end());
    for(auto i: buffer) {
        printf("%c", i);
    }
    printf("\n");
}
int main(int argc, char const *argv[])
{
    // test_mesage();
    test_vector();
    return 0;
}

#endif
