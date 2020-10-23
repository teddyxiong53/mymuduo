#pragma once
#include <vector>

#include "client/player/player.h"
#include <alsa/asoundlib.h>


class AlsaPlayer : public Player
{
public:
    AlsaPlayer(const PcmDevice& pcmDevice, std::shared_ptr<Stream> stream);
    ~AlsaPlayer() override;//注意这个析构函数的override

    void start() override;
    void stop() override;

    static std::vector<PcmDevice> pcm_list();
protected:
    void worker() override;
private:
    bool initAlsa();
    void uninitAlsa();
    snd_pcm_t *m_handle;
    snd_pcm_uframes_t m_frames;
    char *m_buff;
};
