#pragma once

#include "common/endian.h"
#include "common/time_defs.h"
#include <iostream>
#include <streambuf>
#include <sys/time.h>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <stdint.h>
#include "mylog.h"

struct membuf: public std::basic_streambuf<char>
{
    membuf(char *begin, char *end)
    {
        this->setg(begin, begin, end);
    }
};

enum message_type
{
    kBase = 0,
    kCodecHeader = 1,
    kWireChunk = 2,
    kServerSettings = 3,
    kTime = 4,
    kHello = 5,
    kStreamTags = 6,

    kFirst = kBase,
    kLast = kStreamTags
};


struct tv
{
    int32_t sec;
    int32_t usec;

    tv()
    {
        timeval t;
        chronos::systemtimeofday(&t);
        sec = t.tv_sec;
        usec = t.tv_usec;
    }
    tv(timeval tv)
    {
        sec = tv.tv_sec;
        usec = tv.tv_usec;
    }
    tv(int32_t _sec, int32_t _usec)
    {
        sec = _sec;
        usec = _usec;
    }
    tv operator+(const tv& other) const {
        tv result(*this);
        result.sec += other.sec;
        result.usec += other.usec;
        if(result.usec > 1000*1000) {
            result.sec += result.usec/1000000;
            result.usec %= 1000000;
        }
        return result;
    }
    tv operator-(const tv& other) const
    {
        tv result(*this);
        result.sec -= other.sec;
        result.usec -= other.usec;
        while (result.usec < 0)
        {
            result.sec -= 1;
            result.usec += 1000000;
        }
        return result;
    }
};


namespace msg
{
const size_t max_size = 1000*1000;
struct BaseMessage;

using message_ptr = std::shared_ptr<BaseMessage>;

struct BaseMessage
{
    uint16_t type;
    uint16_t id;
    uint16_t refersTo;
    tv received;
    tv sent;
    uint32_t size;

    BaseMessage()
    {
        type = kBase;
        id = 0;
        refersTo = 0;
    }
    BaseMessage(message_type type)
    {
        type = type;
        id = 0;
        refersTo = 0;
    }
    virtual ~BaseMessage() = default;

    //设置为virtual，子类可以选择重写。
    virtual void read(std::istream& stream)
    {
        readVal(stream, type);
        readVal(stream, id);
        readVal(stream, refersTo);
        readVal(stream, sent.sec);
        readVal(stream, sent.usec);
        readVal(stream, received.sec);
        readVal(stream, received.usec);
        readVal(stream, size);

    }
    virtual uint32_t getSize()
    {
        return 3*sizeof(uint16_t)+ 2*sizeof(tv) + sizeof(uint32_t);
    }

    void deserialize(char *payload)
    {
        membuf databuf(payload, payload+BaseMessage::getSize());
        std::istream is(&databuf);
        read(is);
    }
    void deserilized(const BaseMessage& baseMessage, char *payload)
    {
        type = baseMessage.type;
        id = baseMessage.id;
        refersTo = baseMessage.refersTo;
        sent = baseMessage.sent;
        received = baseMessage.received;
        size = baseMessage.size;
        membuf databuf(payload, payload+BaseMessage::getSize());
        std::istream is(&databuf);
        read(is);
    }
    virtual void serialize(std::ostream& stream)
    {
        // mylogd("");
        writeVal(stream, type);
        writeVal(stream, id);
        writeVal(stream, refersTo);
        writeVal(stream, sent.sec);
        writeVal(stream, sent.usec);
        writeVal(stream, received.sec);
        writeVal(stream, received.usec);
        size = getSize();
        writeVal(stream, size);
        // mylogd("tellp:%d", stream.tellp());
        doserialize(stream);
    }
protected:
    void writeVal(std::ostream& stream, const bool& val)
    {
        char c = val? 1: 0;
        writeVal(stream, c);
    }
    void writeVal(std::ostream& stream, const char &val)
    {
        stream.write((const char *)(&val), sizeof(char));
    }
    void writeVal(std::ostream& stream, const int16_t& val )
    {
        int16_t v = SWAP_16(val);
        stream.write((const char *)(&v), sizeof(int16_t));
    }
    void writeVal(std::ostream& stream, const int32_t& val )
    {
        int32_t v = SWAP_32(val);
        stream.write((const char *)(&v), sizeof(int32_t));
    }
    void writeVal(std::ostream& stream, const uint16_t& val )
    {
        uint16_t v = SWAP_16(val);
        stream.write((const char *)(&v), sizeof(uint16_t));
    }
    void writeVal(std::ostream& stream, const uint32_t& val )
    {
        uint32_t v = SWAP_32(val);
        stream.write((const char *)(&v), sizeof(uint32_t));
    }

    void writeVal(std::ostream& stream, const char *payload, const uint32_t size)
    {
        writeVal(stream, size);
        // mylogd("size:%d", size);
        // mylogd("payload:%s", payload);
        stream.write(payload, size);
        // mylogd("tellp:%d", stream.tellp());
    }
    void writeVal(std::ostream& stream, const std::string& val)
    {
        uint32_t size = val.size();
        // mylogd("%s", val.c_str());
        writeVal(stream, val.c_str(), size);
    }

    void readVal(std::istream& stream, bool& val)
    {
        char c;
        readVal(stream, c);
        val = (c!=0);
    }
    void readVal(std::istream& stream, char &val)
    {
        stream.read((char *)(&val), sizeof(char));
    }
    void readVal(std::istream& stream, int16_t val)
    {
        stream.read((char *)(&val), sizeof(int16_t));
        val = SWAP_16(val);
    }
    void readVal(std::istream& stream, int32_t val)
    {
        stream.read((char *)(&val), sizeof(int32_t));
        val = SWAP_32(val);
    }
    void readVal(std::istream& stream, uint16_t val)
    {
        stream.read((char *)(&val), sizeof(uint16_t));
        val = SWAP_16(val);
    }
    void readVal(std::istream& stream, uint32_t val)
    {
        stream.read((char *)(&val), sizeof(uint32_t));
        val = SWAP_32(val);
    }

    void readVal(std::istream& stream, char **payload, uint32_t& size)
    {
        readVal(stream, size);
        *payload = (char *)realloc(*payload, size);
        stream.read(*payload, size);
    }
    void readVal(std::istream& stream, std::string& val)
    {
        uint32_t size;
        readVal(stream, size);
        val.resize(size);
        stream.read(&val[0], size);
    }
    virtual void doserialize(std::ostream& stream)
    {
        //留空。子类可以选择实现。
        mylogd("");
    }
};

struct SerializedMessage
{
    ~SerializedMessage()
    {
        free(buffer);
    }
    BaseMessage message;
    char *buffer;
};


} // namespace msg

