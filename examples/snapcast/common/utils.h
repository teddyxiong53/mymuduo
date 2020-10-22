#pragma once
#include <unistd.h>
#include "string_utils.h"
#include <stdio.h>
#include <sys/utsname.h>
#include <net/if.h>

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

#include <net/ethernet.h>
#include <net/if.h>



#include <netinet/in.h>

#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace strutils = utils::string;


static std::string execGetOutput(const std::string& cmd)
{
    std::shared_ptr<FILE> pipe(
        popen((cmd+" 2> /dev/null").c_str(), "r"),
        pclose
    );
    if(!pipe) {
        return "";
    }
    char buffer[1024];
    std::string result = "";
    while(!feof(pipe.get())) {
        if(fgets(buffer, 1024, pipe.get()) != nullptr) {
            result += buffer;
        }
    }
    return strutils::trim(result);
}

static std::string getHostName()
{
    char hostname[1024] = {0};
    gethostname(hostname, 1023);
    return hostname;
}

static std::string getArch()
{
    std::string arch;
    arch = execGetOutput("arch");
    if(arch.empty()) {
        arch = execGetOutput("uname -i");
    }
    if(arch.empty() || (arch=="unknown")) {
        arch = execGetOutput("uname -m");
    }
    return strutils::trim_copy(arch);
}

static std::string getOS()
{
    std::string os;
    utsname u;
    uname(&u);
    os = u.sysname;
    return strutils::trim_copy(os);
}

static std::string getHostId(const std::string& defaultId)
{
    std::string result = strutils::trim_copy(defaultId);
    if(!result.empty()) {
        return result;
    }
    return getHostName();
}

static std::string getMacAddress(int sock)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[16384] = {0};
    int success = 0;
    if(sock < 0) {
        return "";
    }
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    int ret;
    ret = ioctl(sock, SIOCGIFCONF, &ifc);
    if(ret) {
        return "";
    }
    struct ifreq *it = ifc.ifc_req;
    for(int i=0; i<ifc.ifc_len; ) {
        size_t len = sizeof(*it);
        strcpy(ifr.ifr_name, it->ifr_name);
        if(ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            //跳过loop网卡
            if(!(ifr.ifr_flags & IFF_LOOPBACK)) {
                if(ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        it = (struct ifreq*)((char *)it+len);
        i+=len;
    }
    if(!success) {
        return "";
    }
    char mac[19];
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", (unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[0], (unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[1],
            (unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[2], (unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[3],
            (unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[4], (unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[5]);
    return mac;
}
