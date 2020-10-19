#include "mylog.h"
#include "common/string_utils.h"
#include "client/player/pcm_device.h"
#include <memory>
#include "common/daemon.h"
#include <vector>
#include "client/player/alsa_player.h"

//默认不用daemon
// #define USE_DAEMON


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
    int port = 1704;
    int latency = 0;
    std::unique_ptr<Controller> controller(new Controller(host, instance));
    controller->start(pcmDevice, host, port, latency);
    while(1) {
        sleep(1);
    }
    controller->stop();
    return 0;
}
