#include "stream.h"
#include "mylog.h"
#include "common/time_defs.h"
#include "common/queue.h"
#include "time_provider.h"

Stream::Stream(const SampleFormat& sampleFormat)
 : m_format(sampleFormat)
{
    m_buffer.setSize(500);
    m_shortBuffer.setSize(100);
    m_miniBuffer.setSize(20);
    setRealSampleRate(m_format.rate);
}

void Stream::setRealSampleRate(double sampleRate)
{
    if(sampleRate == m_format.rate) {
        m_correctAfterXFrames = 0;
    } else {
        m_correctAfterXFrames = std::round((m_format.rate / sampleRate) / (m_format.rate / sampleRate - 1.));
    }
}




bool Stream::waitForChunk(size_t ms) const
{
    return true;
}

void Stream::clearChunks()
{
    while(m_chunks.size() > 0) {
        m_chunks.pop();
    }
    resetBuffers();
}

void Stream::resetBuffers()
{
    m_buffer.clear();
    m_miniBuffer.clear();
    m_shortBuffer.clear();
}

void Stream::setBufferLen(size_t bufferLenMs)
{
    m_bufferMs = chronos::msec(bufferLenMs);
}

bool Stream::getPlayerChunk(void *outputBuffer,
    const chronos::usec& outputBufferDacTime,
    unsigned long framesPerBuffer
)
{
    if(outputBufferDacTime > m_bufferMs) {
        mylogd("");
        m_sleep = chronos::usec(0);
        return false;
    }
    if(
        !m_chunk
        || !m_chunks.try_pop(m_chunk, outputBufferDacTime)
    )
    {
        m_sleep = chronos::usec(0);
        return false;
    }
    m_playedFrames += framesPerBuffer;
    /*
    判断一个chunk的时间
    age = 服务器当前时间 - rec_time  - buffer（例如1000ms）+ time_to_dac
    age = 0 现在播放
    age > 0 太老了。
    age < 0 在-age时间后播放。
    */
    chronos::usec age = std::chrono::duration_cast<chronos::usec>(
        TimeProvider::serverNow() -
        m_chunk->start()
    ) - m_bufferMs + outputBufferDacTime;
    if(
        (m_sleep.count() == 0)
        && (chronos::abs(age) > chronos::msec(200))
    )
    {
        mylogd("age > 200ms");
        m_sleep = age;
    }
    try {
        chronos::nsec bufferDuration = chronos::nsec(
            chronos::nsec::rep(
                framesPerBuffer/m_format.nsRate()
            )
        );
        chronos::usec correction = chronos::usec(0);
        if(m_sleep.count() != 0) {
            resetBuffers();

        }
        //TODO 这里代码没写完，看起来很复杂。先放着。
        return true;
    } catch(int e) {
        m_sleep = chronos::usec(0);
        return false;
    }
}

void Stream::addChunk(msg::PcmChunk* chunk)
{
    while(
        m_chunks.size() * chunk->duration<chronos::msec>().count() > 10000
    )
    {
        m_chunks.pop();
    }
    m_chunks.push(std::shared_ptr<msg::PcmChunk>(chunk));
}

