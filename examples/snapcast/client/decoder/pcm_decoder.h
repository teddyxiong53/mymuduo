#pragma once
#include "decoder.h"

namespace decoder
{
class PcmDecoder : public Decoder
{
public:
    PcmDecoder();
    bool decode(msg::PcmChunk *chunk) override;
    SampleFormat setHeader(msg::CodecHeader *chunk) override;
};
} // namespace decoder
