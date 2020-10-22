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
    controller.connect();
    //
    std::string line;
    while(std::getline(std::cin, line)) {
            controller.write(line);
        }
    controller.disconnect();
    return 0;
}
#else
#include "common/message/message.h"
#include "common/message/hello.h"
int main(int argc, char const *argv[])
{
    msg::BaseMessage msg;

    return 0;
}

#endif
