#include "sample_format.h"
#include "mylog.h"
#include <sstream>
#include <vector>
#include "common/utils.h"
#include "common/string_utils.h"
#include "common/str_compat.hpp"

SampleFormat::SampleFormat() = default;

SampleFormat::SampleFormat(const std::string& format)
{
    setFormat(format);
}

SampleFormat::SampleFormat(uint32_t sampleRate,
    uint16_t bitsPerSample,
    uint16_t channelCount
)
{
    setFormat(sampleRate, bitsPerSample, channelCount);
}

std::string SampleFormat::getFormat()
{
    std::stringstream ss;
    ss << rate << ":" << bits << ":" << channels;
    return ss.str();
}

void SampleFormat::setFormat(const std::string& format)
{
    std::vector<std::string> strs;
    strs = utils::string::split(format, ':');
    if(strs.size() == 3) {
        setFormat(cpt::stoul(strs[0]),
            cpt::stoul(strs[1]),
            cpt::stoul(strs[2])
        );
    }
}

void SampleFormat::setFormat(uint32_t rate, uint16_t bits, uint16_t channels)
{
    this->rate =rate;
    this->bits = bits;
    this->channels = channels;
    sampleSize = bits/8;
    if(bits == 24) {
        sampleSize = 4;
    }
    frameSize = channels * sampleSize;
}

