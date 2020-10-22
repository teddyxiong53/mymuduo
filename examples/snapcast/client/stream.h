#pragma once
#include "common/sample_format.h"
#include "common/time_defs.h"
#include "client/double_buffer.h"

class Stream
{
public:
    Stream(const SampleFormat& format);
    bool getPlayerChunk(void *outputBuffer,
        const chronos::usec& outputBufferDacTime,
        unsigned long framesPerBuffer
    );

    const SampleFormat& getFormat() {
        return m_format;
    }
    bool waitForChunk(size_t ms) const;
    void clearChunks();
    void setRealSampleRate(double rate);
private:
    SampleFormat m_format;
    DoubleBuffer<chronos::usec::rep> m_miniBuffer;
    DoubleBuffer<chronos::usec::rep> m_buffer;
    DoubleBuffer<chronos::usec::rep> m_shortBuffer;
    long m_correctAfterXFrames;
    chronos::msec m_bufferMs;
    
};

