#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <condition_variable>
#include <thread>
#include <mutex>
#include "decoderImp.h"
#include "alsaOut.h"

typedef enum{
    UNKNOWN,
    PCM,
    WAVE,
    MP3,
    OPUS,
} audioType;

typedef enum {
    NONE,
    START,
    PAUSE,
    STOP,
    SEEK,
} requestState;

typedef enum {
    IDLE,
    RUNNING,
    PAUSING,
    SEEKING,
    STOPPING,
    STOPPED,
} playState;


class audioplayer
{
    public:
        audioplayer();
        ~audioplayer();

        bool Init(const char* fname);
        int start();
        int pause();
        int seek(int sec);
        int stop();
        int getDuration();

    private:
        int seek_sec;
        int duration;
        alsaOut *alsaout;
        decoderImp *mdecoder;
        audioType mAudioType;
        volatile playState plyState;
        volatile requestState reqState;
        struct audioParams aparam;

        std::thread playThread;
        std::mutex mtxPlayState;
        std::condition_variable cvPlayState;
        std::condition_variable cvPauseState;

        void PlaybackThread(void);
        static audioType getAudioType(const char* fname);




};

#endif /* _PLAYER_H_ */

