#pragma once
#include "common/sample_format.h"
#include "common/time_defs.h"

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
private:
    SampleFormat m_format;
};

