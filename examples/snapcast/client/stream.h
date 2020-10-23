#pragma once
#include "common/sample_format.h"
#include "common/time_defs.h"
#include "client/double_buffer.h"
#include "common/queue.h"
#include <chrono>
#include "common/message/pcm_chunk.h"

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
    void setBufferLen(size_t bufferLenMs);
    void resetBuffers();

private:
    SampleFormat m_format;
    chronos::usec m_sleep;
    Queue<std::shared_ptr<msg::PcmChunk>> m_chunks;
    DoubleBuffer<chronos::usec::rep> m_miniBuffer;
    DoubleBuffer<chronos::usec::rep> m_buffer;
    DoubleBuffer<chronos::usec::rep> m_shortBuffer;
    long m_correctAfterXFrames;
    chronos::msec m_bufferMs;
    std::shared_ptr<msg::PcmChunk> m_chunk;
    int m_median;
    int m_shortMedian;
    time_t m_lastUpdate;
    unsigned long m_playedFrames;

};

