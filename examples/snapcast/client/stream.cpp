#include "stream.h"
#include "mylog.h"

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

