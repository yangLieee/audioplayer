#include <stdio.h>
#include "alsaOut.h"
#include "mlog.h"
#define MLOG_LEVEL DEBUG

alsaOut::alsaOut()
{
    int _channels = 2;
    int samplerate = 44100;
    buffer_size = SAMPLE_BUFSIZE;
};

alsaOut::~alsaOut()
{

}

int alsaOut::openDevice(const char *devName, int channels, int samplerate)
{
    _devName = devName;
    _channels = channels;
    _samplerate = samplerate;
    if(setupHandle())  {
        MLOGE("open Device Failed!");
        return -1;
    }
    MLOGI("open %s Device successful!", devName);
    return 0;
}

int alsaOut::setupHandle()
{
    int ret = -1;
    if(alsaHandle) {
        snd_pcm_close(alsaHandle);
        alsaHandle = NULL;
    }
    ret = snd_pcm_open(&alsaHandle, _devName , SND_PCM_STREAM_PLAYBACK, 0);
    if(ret < 0) {
        MLOGE("Failed open default playback device");
        return -1;
    }
    snd_pcm_hw_params_t *hwparams;
    /* 初始化参数空间 */
    snd_pcm_hw_params_malloc(&hwparams);
    if(hwparams != NULL) {
        /* 使用默认数据填充对象 */
        snd_pcm_hw_params_any(alsaHandle, hwparams);
        /* interleaved mode */
        snd_pcm_hw_params_set_access(alsaHandle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
        /* Signed 16bit little endian format */
        snd_pcm_hw_params_set_format(alsaHandle, hwparams, SND_PCM_FORMAT_S16_LE);
        /* set channels */
        snd_pcm_hw_params_set_channels(alsaHandle, hwparams, _channels);
        /* set samplerate near hw supported */
        snd_pcm_hw_params_set_rate_near(alsaHandle, hwparams, &_samplerate, 0);
        /* set max buffer size for hw */
#ifdef MAX_BUFFER_SIZE
        snd_pcm_hw_params_get_buffer_size_max(hwparams, &buffer_size);
        snd_pcm_hw_params_set_buffer_size_near(alsaHandle, hwparams, &buffer_size);
#else
        buffer_size = SAMPLE_BUFSIZE;
        snd_pcm_hw_params_set_buffer_size_near(alsaHandle, hwparams, &buffer_size);
#endif
        /* set 1/4 buffer_size as period_size */
        period_size = buffer_size / 4;
        snd_pcm_hw_params_set_period_size_near(alsaHandle, hwparams, &period_size, 0);
        /* write hwparams to driver */
        ret = snd_pcm_hw_params(alsaHandle, hwparams);
        if(ret < 0) {
            MLOGE("Failed set hw params!");
            snd_pcm_close(alsaHandle);
            alsaHandle = NULL;
            return -1;
        }
        ret = snd_pcm_prepare(alsaHandle);
        if(ret < 0) {
            MLOGE("Failed prepare audio interface for use\n");
            return -1;  
        }  
        snd_pcm_hw_params_free(hwparams);  
        MLOGI("Set params to hw successful! channel: %d, samplerate: %d", _channels,_samplerate);
    }
    else {
        MLOGE("Failed alloca space!");
        return -1;
    }
    return 0;
}

int alsaOut::playback(uint16_t *samples, int nb_samples)
{
    int tryCount = 3;
    int nb_frame = nb_samples;
retry:
    int retcode = snd_pcm_writei(alsaHandle, samples, nb_frame);
    if(retcode == -EBADFD) {
        MLOGE("	PCM is not in the right state (SND_PCM_STATE_PREPARED or SND_PCM_STATE_RUNNING)");
        usleep(10);
    }
    else if(retcode == -EPIPE) {
        MLOGE("an underrun occurred ");
        snd_pcm_prepare(alsaHandle);
        tryCount--;
        if(tryCount > 0)
            goto retry;
    }
    else if(retcode == -ESTRPIPE) {
        MLOGE("a suspend event occurred (stream is suspended and waiting for an application recovery)");
        tryCount--;
        if(tryCount > 0)
            goto retry;
    }
    else if(retcode != nb_frame) {
        samples += retcode;
        nb_frame -= retcode;
        tryCount--;
        if(tryCount > 0)
            goto retry;
    }
    return nb_samples;
}
