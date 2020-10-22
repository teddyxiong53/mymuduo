#pragma once

#include "message.h"
namespace msg
{
class Time : public BaseMessage
{
public:
    Time() : BaseMessage(message_type::kTime)
    {

    }
    ~Time() override = default;
    void read(std::istream& stream) override
    {
        readVal(stream, latency.sec);
        readVal(stream, latency.usec);
    }
    uint32_t getSize()  override
    {
        return sizeof(tv);
    }
    tv latency;
protected:
    void doserialize(std::ostream& stream)  override
    {
        writeVal(stream, latency.sec);
        writeVal(stream, latency.usec);
    }
};

} // namespace msg

