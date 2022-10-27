#ifndef _ALSAOUT_H_
#define _ALSAOUT_H_

#include <alsa/asoundlib.h>
#define SAMPLE_BUFSIZE 8192

class alsaOut
{
    public:
        alsaOut();
        ~alsaOut();
        int openDevice(const char *devName, int channels, int samplerate);
        int playback(uint16_t *samples, int nb_samples);
    private:
        int setupHandle();
        const char *_devName;
        int _channels;
        unsigned int _samplerate;
        snd_pcm_t *alsaHandle;
        snd_pcm_uframes_t buffer_size;
        snd_pcm_uframes_t period_size;


};

#endif /* _ALSAOUT_H_ */
