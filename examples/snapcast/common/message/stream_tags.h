#pragma once
#include "json_message.h"
namespace msg
{
/*
    格式是这样：
    {
        "artist": "xxx",
        "album": "xx",
        "track": "xx",
        //...
    }
*/
class StreamTags : public JsonMesasge
{
public:
    StreamTags(json j) : JsonMesasge(message_type::kStreamTags)
    {
        msg = j;
    }
    StreamTags() : JsonMesasge(message_type::kStreamTags)
    {
        
    }
    ~StreamTags() override = default;
};

} // namespace msg
