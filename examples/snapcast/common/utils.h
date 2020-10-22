#pragma once
#include <unistd.h>
#include "string_utils.h"
#include <stdio.h>
#include <sys/utsname.h>
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

