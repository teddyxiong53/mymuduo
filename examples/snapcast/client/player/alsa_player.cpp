#include "alsa_player.h"
#include "mylog.h"
#include "client/stream.h"
#include "common/time_defs.h"


#define PERIOD_TIME (30*1000)//30ms

AlsaPlayer::AlsaPlayer(
    const PcmDevice& pcmDevice,
    std::shared_ptr<Stream> stream
)
 : Player(pcmDevice, stream),
   m_handle (nullptr),
   m_buff(nullptr)
{

}

bool AlsaPlayer::initAlsa()
{
    int ret = 0, channels=0;

    snd_pcm_hw_params_t *params = nullptr;
    const SampleFormat& format = m_stream->getFormat();
    unsigned int rate = format.rate;
    snd_pcm_format_t snd_pcm_format = (snd_pcm_format_t)0;
    unsigned int period_time = 0;
    int buff_size = 0;
    snd_pcm_sw_params_t *swparams = nullptr;
    unsigned int buffer_time = 0;
    ret = snd_pcm_open(&m_handle, m_pcmDevice.name.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if(ret < 0) {
        myloge("open fail");
        goto err1;
    }
    //设置hw params

    snd_pcm_hw_params_alloca(&params);
    //是在栈上分配的空间，不需要手动释放。
    ret = snd_pcm_hw_params_any(m_handle, params);
    if(ret < 0) {
        myloge("fill params fail");
        goto err2;
    }
    //设置为交叉模式
    ret = snd_pcm_hw_params_set_access(m_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if(ret < 0) {
        myloge("set interleaved fail");
        goto err2;
    }
    //下面设置格式
    //先从stream里拿到格式


    if(format.bits == 8) {
        snd_pcm_format = SND_PCM_FORMAT_S8;
    } else if(format.bits == 16) {
        snd_pcm_format = SND_PCM_FORMAT_S16;
    } else if(format.bits == 32) {
        snd_pcm_format = SND_PCM_FORMAT_S32;
    } else {
        myloge("unsupported format: ");
        goto err2;
    }
    ret = snd_pcm_hw_params_set_format(m_handle, params, snd_pcm_format);
    if(ret < 0) {
        myloge("set format fail");
        goto err2;
    }
    //设置channels
    ret = snd_pcm_hw_params_set_channels(m_handle, params, format.channels);
    if(ret < 0) {
        myloge("set channels fail");
        goto err2;
    }
    //设置采样率

    ret = snd_pcm_hw_params_set_rate_near(m_handle, params, &rate, nullptr);
    if(ret < 0) {
        myloge("set rate fail");
        goto err2;
    }
    mylogd("real rate:%d", rate);

    snd_pcm_hw_params_get_period_time_max(params, &period_time, nullptr);
    mylogd("period time max:%d", period_time);
    if(period_time > PERIOD_TIME) {
        mylogd("adjust period time to 30ms");
        period_time = PERIOD_TIME;
    }
    //设置period time和buffer time
    snd_pcm_hw_params_set_period_time_near(m_handle, params, &period_time, nullptr);
    buffer_time = 4*period_time;
    snd_pcm_hw_params_set_period_time_near(m_handle, params, &buffer_time, nullptr);

    //写入参数
    ret = snd_pcm_hw_params(m_handle, params);
    if(ret < 0) {
        myloge("write params fail");
        goto err2;
    }
    mylogd("pcm name:%s\n", snd_pcm_name(m_handle));
    mylogd("pcm state:%s\n", snd_pcm_state_name(snd_pcm_state(m_handle)));
    //获取period size，我们只设置了时间，size要靠内部计算出来。
    //我们要用这个size来分配一段内存。
    //先拿到一个period有多少个frame
    snd_pcm_hw_params_get_period_size(params, &m_frames, nullptr);
    mylogd("frames:%d", m_frames);

    buff_size = m_frames * format.frameSize;
    m_buff = (char *)malloc(buff_size);
    if(!m_buff) {
        myloge("malloc fail");
        goto err2;
    }

    //设置swparams

    snd_pcm_sw_params_alloca(&swparams);
    //先获取当前的配置
    snd_pcm_sw_params_current(m_handle, swparams);
    //设置
    snd_pcm_sw_params_set_avail_min(m_handle, swparams, m_frames);
    snd_pcm_sw_params_set_start_threshold(m_handle, swparams, m_frames);
    //写入
    snd_pcm_sw_params(m_handle, swparams);

    return true;
err2:
    snd_pcm_close(m_handle);
    m_handle = nullptr;

err1:
    return false;
}

void AlsaPlayer::uninitAlsa()
{
    if(m_handle != nullptr) {
        snd_pcm_drain(m_handle);
        snd_pcm_close(m_handle);
        m_handle = nullptr;
    }
    if(m_buff != nullptr) {
        free(m_buff);
        m_buff = nullptr;
    }
}


void AlsaPlayer::start()
{
    initAlsa();
    Player::start();

}

void AlsaPlayer::stop()
{
    Player::stop();
    uninitAlsa();
}

AlsaPlayer::~AlsaPlayer()
{
    stop();
}

void AlsaPlayer::worker()
{
    bool bool_ret;
    snd_pcm_sframes_t pcm;
    snd_pcm_sframes_t framesDelay;
    int ret;
    long lastChunkTick = chronos::getTickCount();
    while(m_active) {
        if(m_handle == nullptr) {
            bool_ret = initAlsa();
            if(!bool_ret) {
                chronos::sleep(100);//睡100ms再试。
            }
        }
        //获取延迟
        snd_pcm_delay(m_handle, &framesDelay);
        chronos::usec delay(
            (chronos::usec::rep)
            ((1000*(double)framesDelay)/m_stream->getFormat().msRate())
        );
        //
        if(m_stream->getPlayerChunk(m_buff, delay, m_frames)) {
            lastChunkTick = chronos::getTickCount();
            adjustVolume(m_buff, (size_t)m_frames);
            ret = snd_pcm_writei(m_handle, m_buff, m_frames);
            if(ret == -EPIPE) {
                myloge("xrun");
                snd_pcm_prepare(m_handle);
            } else if(ret < 0) {
                myloge("fatal error:%s", snd_strerror(ret));
                uninitAlsa();
            }
        } else {
            mylogd("fail to get chunk");
            while(m_active && !m_stream->waitForChunk(100)) {
                mylogd("waiting for chunk");
                if(m_handle != nullptr &&
                    (chronos::getTickCount()-lastChunkTick > 5000)
                )
                {
                    mylogd("no chunk received in 5000ms, closing alsa");
                    uninitAlsa();
                    m_stream->clearChunks();
                }
            }
        }
    }
}

std::vector<PcmDevice> AlsaPlayer::pcm_list()
{
    void **hints, **n;
    int ret;
    ret = snd_device_name_hint(-1, "pcm", &hints);
    n = hints;
    char *name, *desc, *io;
    std::vector<PcmDevice> result;
    PcmDevice pcmDevice;
    while(*n != nullptr) {
        name = snd_device_name_get_hint(*n, "NAME");
        desc = snd_device_name_get_hint(*n, "DESC");
        io = snd_device_name_get_hint(*n, "IOID");
        if(io!=nullptr && strcmp(io, "Output")!=0) {
            goto __end;
        }
        pcmDevice.name = name;
        if(desc == nullptr) {
            pcmDevice.description = "";
        } else {
            pcmDevice.description = desc;
        }
        pcmDevice.idx++;
        result.push_back(pcmDevice);
    __end:
        if(name!=nullptr) {
            free(name);
        }
        if(desc != nullptr) {
            free(desc);
        }
        if(io != nullptr) {
            free(io);
        }
        n++;
    }
    snd_device_name_free_hint(hints);
    return result;
}

