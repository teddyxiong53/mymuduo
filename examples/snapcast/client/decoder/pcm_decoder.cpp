#include "pcm_decoder.h"
#include "mylog.h"
#include "common/snap_exception.h"

namespace decoder
{
#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT 0x20746d66
#define ID_DATA 0x61746164


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
        throw SnapException("pcm header too small");
    }
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt;
    chunk_fmt.sample_rate = SWAP_32(0);
    chunk_fmt.bits_per_sample = SWAP_16(0);
    chunk_fmt.num_channels = SWAP_16(0);

    size_t pos = 0;
    memcpy(&riff_wave_header, chunk->payload + pos, sizeof(riff_wave_header));
    pos += sizeof(riff_wave_header);

    if(
        (SWAP_32(riff_wave_header.riff_id) != ID_RIFF)
        && (SWAP_32(riff_wave_header.riff_id) != ID_WAVE)
    )
    {
        myloge("fmt is not wave or riff");
        throw SnapException("fmt is not wave or riff");
    }
    bool moreChunks = true;
    do {
        if(pos + sizeof(chunk_header) > chunk->payloadSize) {
            throw SnapException("riff/wave header not complete");
        }
        memcpy(&chunk_header, chunk->payload+pos, sizeof(chunk_header));
        pos += sizeof(chunk_header);
        switch(SWAP_32(chunk_header.id)) {
            case ID_FMT:
                if(pos + sizeof(chunk_fmt) > chunk->payloadSize) {
                    throw SnapException("riff/wave header not complete");
                }
                memcpy(&chunk_fmt, chunk->payload+pos, sizeof(chunk_fmt));
                pos += sizeof(chunk_fmt);
                //如果头部多了，跳过多余的部分
                if(SWAP_32(chunk_header.sz) > sizeof(chunk_fmt)) {
                    pos += (SWAP_32(chunk_header.sz) - sizeof(chunk_fmt));
                }
                break;
            case ID_DATA:
                moreChunks = false;
                break;
            default:
                pos += SWAP_32(chunk_header.sz);
        }
    } while(moreChunks);
    if(SWAP_32(chunk_fmt.sample_rate) == 0) {
        throw SnapException("sample format not found");
    }
    SampleFormat sampleFormat(
        SWAP_32(chunk_fmt.sample_rate),
        SWAP_16(chunk_fmt.bits_per_sample),
        SWAP_16(chunk_fmt.num_channels)
    );
    return sampleFormat;
}

} // namespace decoder
