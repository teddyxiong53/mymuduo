#pragma once
#include "common/message/pcm_chunk.h"
#include "common/sample_format.h"
#include "common/message/codec_header.h"
#include <mutex>

namespace decoder
{
class Decoder
{
public:
    Decoder()
    {

    }
    virtual ~Decoder() = default;
    virtual bool decode(msg::PcmChunk* chunk) = 0;
    virtual SampleFormat setHeader(msg::CodecHeader *chunk)= 0;
protected:
    std::mutex m_mutex;
};
} // namespace decoder
