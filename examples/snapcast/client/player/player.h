#pragma once
#include "pcm_device.h"
#include <memory>
#include <string>
#include <atomic>
#include "client/stream.h"
#include <thread>
#include "common/endian.h"
class Player
{
public:
    Player(const PcmDevice& pcmDevice, std::shared_ptr<Stream> stream);
    virtual ~Player();

    virtual void setVolume(double volume);
    virtual void setMute(bool mute);
    virtual void start();
    virtual void stop();

protected://下面的内容都会被子类继承，包括变量。
    virtual void worker() = 0;
    void setVolume_poly(double volume, double exp);
    void setVolume_exp(double volume, double base);

    template<typename T>
    void adjustVolume(char *buffer, size_t count, double volume)
    {
        T* bufferT = (T*)buffer;
        for(size_t n=0; n<count; n++) {
            bufferT[n] = endian::swap<T>(endian::swap<T>(bufferT[n]) * volume);
        }
    }
    void adjustVolume(char *buffer, size_t frames);
    std::atomic<bool> m_active;
    std::shared_ptr<Stream> m_stream;
    std::thread m_playerThread;
    PcmDevice m_pcmDevice;
    double m_volume;
    bool m_mute;
    double m_volCorrection;
private:

};
