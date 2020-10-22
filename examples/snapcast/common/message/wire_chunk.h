#pragma once
#include "common/message/message.h"

namespace msg
{
class WireChunk : public BaseMessage
{
public:
    tv timestamp;
    uint32_t payloadSize;
    char *payload;
    WireChunk(size_t size=0)
    : BaseMessage(message_type::kWireChunk),
      payloadSize(size),
      payload(nullptr)
    {
        if(payload > 0) {
            payload = (char *)malloc(size);
        }
    }
    WireChunk(const WireChunk& wireChunk)
    : BaseMessage(message_type::kWireChunk)
    {
        timestamp = wireChunk.timestamp;
        payloadSize = wireChunk.payloadSize;
        payload = (char *)malloc(payloadSize);
        memcpy(payload, wireChunk.payload, payloadSize);
    }
    ~WireChunk() override
    {
        free(payload);
        payload = nullptr;
    }
    void read(std::istream& stream) override
    {
        readVal(stream, timestamp.sec);
        readVal(stream, timestamp.usec);
        readVal(stream, &payload, payloadSize);
    }
    uint32_t getSize() override
    {
        return sizeof(tv) + sizeof(uint32_t) + payloadSize;
    }

protected:
    void doserialize(std::ostream& stream) override
    {
        writeVal(stream, timestamp.sec);
        writeVal(stream, timestamp.usec);
        writeVal(stream, payload, payloadSize);
    }
private:

};
} // namespace msg

