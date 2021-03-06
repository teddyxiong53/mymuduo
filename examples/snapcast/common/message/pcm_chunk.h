#pragma once

#include "common/sample_format.h"
#include "message.h"
#include "wire_chunk.h"

namespace msg
{
class PcmChunk: public WireChunk
{
public:
    SampleFormat format;//public的，不 以 m_开头。
    PcmChunk(const SampleFormat& sampleFormat, size_t ms)
    : WireChunk(sampleFormat.rate* sampleFormat.frameSize*ms/1000),
      format(sampleFormat),
      m_idx(0)
    {

    }
    PcmChunk(const PcmChunk& pcmChunk)
    : WireChunk(pcmChunk),
      format(pcmChunk.format),
      m_idx(0)//这个还是0
    {

    }
    ~PcmChunk() override = default;

    int readFrames(void *outputBuffer, size_t frameCount)
    {
        int result = frameCount;
        if(m_idx + frameCount > (payloadSize/format.frameSize)) {
            result = (payloadSize/format.frameSize) - m_idx;
        }
        if(outputBuffer != nullptr) {
            memcpy((char *)outputBuffer, (char *)(payload) + format.frameSize *m_idx, format.frameSize*result);
        }
        m_idx += result;
        return result;
    }

    chronos::time_point_clk start() override
    {
        return chronos::time_point_clk(
            chronos::sec(timestamp.sec) + chronos::usec(timestamp.usec)
            +
            chronos::usec((chronos::usec::rep)(1000000. * ((double)m_idx / (double)format.rate)))
        );
    }
    inline chronos::time_point_clk end()
    {
        return start() + durationLeft<chronos::usec>();
    }
    template <typename T>
    inline T durationLeft()
    {
        return std::chrono::duration_cast<T>(
            (chronos::nsec((chronos::nsec::rep)(1000000 * (getFrameCount() - m_idx) / format.msRate())))
        );
    }
    inline size_t getFrameCount()
    {
        return (payloadSize/format.frameSize);
    }
    inline size_t getSampleCount()
    {
        return (payloadSize/format.sampleSize);
    }
    inline bool isEndOfChunk()
    {
        return m_idx >= getFrameCount();
    }
    template <typename T>
    inline T duration() 
    {
        return std::chrono::duration_cast<T>(chronos::nsec((chronos::nsec::rep)(1000000 * getFrameCount() / format.msRate())));
    }
protected:
private:
    uint32_t m_idx;
};
} // namespace msg
