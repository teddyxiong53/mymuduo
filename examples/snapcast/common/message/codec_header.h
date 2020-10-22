#pragma once

#include "common/message/message.h"

namespace msg
{
class CodecHeader: public BaseMessage
{
public:
    uint32_t payloadSize;
    char *payload;
    std::string codec;
    CodecHeader(const std::string& codecName="", size_t size=0)
    : BaseMessage(message_type::kCodecHeader),
      payloadSize(size),
      payload(nullptr),
      codec(codecName)
    {
        if(payloadSize > 0) {
            payload = (char *)malloc(payloadSize);
        }
    }
    ~CodecHeader() override
    {
        if(payload) {
            free(payload);
            payload = nullptr;
        }
    }
    uint32_t getSize() override
    {
        return sizeof(uint32_t) + codec.size() + sizeof(uint32_t) + payloadSize;
    }
    void read(std::istream& stream) override
    {
        readVal(stream, codec);
        readVal(stream, &payload, payloadSize);
    }
protected:
    void doserialize(std::ostream& stream) override
    {
        writeVal(stream, codec);
        writeVal(stream, payload, payloadSize);
    }
};
} // namespace msg

