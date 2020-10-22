#include "stream.h"
#include "mylog.h"


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


bool Stream::getPlayerChunk(void *outputBuffer,
    const chronos::usec& outputBufferDacTime,
    unsigned long framesPerBuffer
)
{
    return true;
}


bool Stream::waitForChunk(size_t ms) const
{
    return true;
}

void Stream::clearChunks()
{

}

