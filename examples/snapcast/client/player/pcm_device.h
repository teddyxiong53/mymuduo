#pragma once

#include <string>

struct PcmDevice {
    PcmDevice(): idx(-1), name("default"){}
    PcmDevice(int idx, const std::string& name, const std::string& description="")
     : idx(idx), name(name), description(description)
    {

    }
    int idx;
    std::string name;
    std::string description;
};
