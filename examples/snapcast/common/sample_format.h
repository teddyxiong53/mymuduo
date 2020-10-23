#pragma once
#include <cstdint>
#include <string>

class SampleFormat
{
public:
    SampleFormat();
    SampleFormat(const std::string& format);
    SampleFormat(uint32_t rate, uint16_t bits, uint16_t channels);

    uint32_t rate;
    uint16_t bits;
    uint16_t channels;
    uint16_t sampleSize;
    uint16_t frameSize;

    std::string getFormat() ;

    void setFormat(const std::string& format);
    void setFormat(uint32_t rate, uint16_t bits, uint16_t channels);

    inline double msRate() const {
        return (double)rate/1000.0;
    }
    inline double usRate() const {
        return (double)rate/1000000.0;
    }
    inline double nsRate() const {
        return (double)rate/1000000000.0;
    }
private:
};
