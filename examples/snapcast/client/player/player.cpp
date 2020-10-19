#include "player.h"
#include "common/sample_format.h"
#include <math.h>
#include "mylog.h"

Player::Player(
    const PcmDevice& pcmDevice,
    std::shared_ptr<Stream> stream
)
 : m_active(false),
   m_stream(stream),
   m_pcmDevice(pcmDevice),
   m_mute(false),
   m_volume(1.0),
   m_volCorrection(1.0)
{

}

Player::~Player()
{
    stop();
}

void Player::start()
{
    m_active = true;
    m_playerThread = std::thread(&Player::worker, this);
}

void Player::stop()
{
    if(m_active) {
        m_active = false;
        m_playerThread.join();
    }
}

void Player::setMute(bool mute)
{
    m_mute = mute;
}

void Player::setVolume(double volume)
{
    setVolume_exp(volume, 10.0);
}

void Player::setVolume_exp(double volume, double base)
{
    m_volume = (pow(base, volume)-1)/(base-1);
    mylogd("set volume exp: %f", m_volume);
}

void Player::adjustVolume(char *buffer, size_t frames)
{
    double volume = m_volume;
    if(m_mute) {
        volume = 0;
    }
    const SampleFormat& sampleFormat = m_stream->getFormat();
    if(
        (volume < 1.0)
        || (m_volCorrection != 1.0)
    )
    {
        //上面的调节是避免不必要的计算。
        volume *= m_volCorrection;
        if(sampleFormat.sampleSize == 1) {
            adjustVolume<int8_t>(buffer, frames* sampleFormat.channels, volume);
        } else if(sampleFormat.sampleSize == 2) {
            adjustVolume<int16_t>(buffer, frames* sampleFormat.channels, volume);
        } else if(sampleFormat.sampleSize == 4) {
            adjustVolume<int32_t>(buffer, frames* sampleFormat.channels, volume);
        }
    }
}
