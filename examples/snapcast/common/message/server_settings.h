#pragma once
#include "json_message.h"


namespace msg
{
/*
    有4个设置：
    buffer
    延迟
    音量
    mute
*/
class ServerSettings: public JsonMesasge
{

public:
    ServerSettings() : JsonMesasge(message_type::kServerSettings)
    {
        setBufferMs(0);
        setLatency(0);
        setVolume(100);
        setMuted(false);
    }
    ~ServerSettings() override = default;
    void setBufferMs(int32_t ms)
    {
        msg["bufferMs"] = ms;
    }
    void setLatency(int32_t latency)
    {
        msg["latency"] = latency;
    }
    void setVolume(uint16_t volume)
    {
        msg["volume"] = volume;
    }
    void setMuted(bool muted)
    {
        msg["muted"] = muted;
    }
        int32_t getBufferMs()
    {
        return get("bufferMs", 0);
    }

    int32_t getLatency()
    {
        return get("latency", 0);
    }

    uint16_t getVolume()
    {
        return get("volume", 100);
    }

    bool isMuted()
    {
        return get("muted", false);
    }

};



} // namespace msg

