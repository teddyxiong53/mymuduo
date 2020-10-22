#include "pcm_decoder.h"
#include "mylog.h"

namespace decoder
{

struct riff_wave_header
{
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header
{
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt
{
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};


PcmDecoder::PcmDecoder()
 : Decoder()
{

}
/*
    不用解码。直接就可以用的。
*/
bool PcmDecoder::decode(msg::PcmChunk *chunk)
{
    return true;
}

SampleFormat PcmDecoder::setHeader(msg::CodecHeader* chunk)
{
    //44是wav的头部的长度。
    if(chunk->payloadSize < 44) {
        myloge("pcm header too small");
        //不抛异常，怎么返回？
    }
    
}
} // namespace decoder
