#include <stdio.h>
#include <string.h>
#include "player.h"
#include "wavDecoder.h"
#include "mlog.h"

#define MLOG_LEVEL DEBUG

audioplayer::audioplayer()
{
    seek_sec = 0;
    duration = 0;
    alsaout = NULL;
    mdecoder = NULL;
    plyState = STOPPED;
    reqState = NONE;
    mAudioType = UNKNOWN;
    memset(&aparam, 0, sizeof(audioParams));
}

audioplayer::~audioplayer()
{
}

bool audioplayer::Init(const char* fname)
{
    mAudioType = audioplayer::getAudioType(fname);
    switch(mAudioType) {
        case WAVE:
            mdecoder = wavdecoder::getInstance(fname, &aparam);
            break;
        case MP3:
//            mdecoder = mp3decoder::getInstance(fname);
            break;
    }
    alsaout = new alsaOut();
    alsaout->openDevice("default", aparam.channels, aparam.samplerate);
    playThread = std::thread(&audioplayer::PlaybackThread, this);
    playThread.detach();
    plyState = IDLE;
    return true;
}

int audioplayer::start()
{
    MLOGI("Enter start!");
    std::unique_lock<std::mutex> lck(mtxPlayState);
    if(plyState == IDLE || plyState == PAUSING) {
        reqState = START;
        cvPauseState.notify_one();
        cvPlayState.wait(lck, [this](){return (reqState != START && plyState == RUNNING);});
        MLOGI("Start ok!");
    }
    else {
        MLOGW("Operation is not permitted!");
        return -1;
    }
    return 0;
}

int audioplayer::pause()
{
    MLOGI("Enter Pause!");
    std::unique_lock<std::mutex> lck(mtxPlayState);
    if(plyState == RUNNING) {
        reqState = PAUSE;
        cvPlayState.wait(lck, [this](){return (reqState != PAUSE && plyState == PAUSING);});
        MLOGI("Pause ok!");
    }
    else {
        MLOGW("Operation is not permitted");
        return -1;
    }
    return 0;
}

int audioplayer::seek(int sec)
{
    MLOGI("Enter seek!");
    if(sec > duration || sec < 0) {
        MLOGW("Please input seek_time less than %ds",duration);
        return -1;
    }
    std::unique_lock<std::mutex> lck(mtxPlayState);
    /* This tmp state is recoder preState for resume state */
    playState prePlyState;
    if(plyState == RUNNING || plyState == PAUSING) {
        prePlyState = plyState;
        reqState = SEEK;
        seek_sec = sec;
        cvPauseState.notify_one();
        cvPlayState.wait(lck, [this](){return (reqState != SEEK && plyState == SEEKING);});
        MLOGI("Seek ok!");
        plyState = prePlyState;
    }
    else {
        MLOGW("Operation is not permitted");
        return -1;
    }
    return 0;
}

int audioplayer::stop()
{
    MLOGI("Enter Stop!");
    std::unique_lock<std::mutex> lck(mtxPlayState);
    if(plyState == STOPPED) {
        MLOGW("Playback thread is already stopped!");
        return -1;
    }
    else {
        reqState = STOP;
        cvPauseState.notify_one();
        cvPlayState.wait(lck, [this](){return (reqState != STOP && plyState == STOPPED);});
        MLOGI("Stop ok!");
    }
    return 0;
}

int audioplayer::getDuration()
{
    duration = mdecoder->getDuration();
    return duration;
}

void audioplayer::PlaybackThread()
{
    unsigned char *sbuffer = NULL;
    int dsize, frame_size;
    while(1) {
        {
            std::unique_lock<std::mutex> lck(mtxPlayState);
            switch(reqState) {
                case START:
                    reqState = NONE;
                    plyState = RUNNING;
                    cvPlayState.notify_one();
                    break;
                case PAUSE:
                    reqState = NONE;
                    plyState = PAUSING;
                    cvPlayState.notify_one();
                    cvPauseState.wait(lck, [this](){return (reqState == START || reqState == SEEK  || reqState == STOP);});
                    break;
                case SEEK:
                    reqState = NONE;
                    plyState = SEEKING;
                    cvPlayState.notify_one();
                    mdecoder->seek(seek_sec);
                    break;
                case STOP:
                    reqState = NONE;
                    plyState = STOPPING;
                    mdecoder->stop();
                    /* TO DO */
                    /* alsaout->stop(); */
                    break;
                default:
                    break;
            }
        }
        if(plyState == RUNNING) {
            sbuffer = (unsigned char *)mdecoder->decode(&dsize);
            /* frame_size = all_size / byte_per_samples */
            frame_size = dsize / aparam.channels / 2;
            alsaout->playback((uint16_t *)sbuffer, frame_size);
        }
        if(plyState == STOPPING)
            break;
    }
    plyState = STOPPED;
    cvPlayState.notify_one();
    MLOGI("Main play thread will exit!");
}

audioType audioplayer::getAudioType(const char* fname)
{
    const char *sub = strrchr(fname, '.');
    const char *typchr = sub + 1;
    if(!strncmp(typchr, "raw", 3))
        return  PCM;
    else if(!strncmp(typchr, "wav", 3))
        return WAVE;
    else if(!strncmp(typchr, "mp3", 3))
        return MP3;
    else if(!strncmp(typchr, "opus", 4))
        return OPUS;
    else 
        return UNKNOWN;
}

