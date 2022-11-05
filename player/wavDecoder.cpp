#include <stdio.h>
#include <string.h>
#include "wavDecoder.h"
#include "mlog.h"

#define MLOG_LEVEL DEBUG

wavdecoder::wavdecoder()
{
    fp = NULL;
    dataSize = 0;
    fpos = 0;
    memset(&head, 0, sizeof(wavHead));
    memset(rbuf, 0, sizeof(rbuf));
}

wavdecoder::~wavdecoder()
{
    fclose(fp);
    fp = NULL;
}

bool wavdecoder::Init(const char* fname, audioParams *aparam)
{
    fp = fopen(fname, "rb");
    if(fp == NULL){
        MLOGE("Failed open %s!", fname);
        return false;
    }
    wave_get_header(fp, &head);
    dataSize = wave_get_fsize(fp) - sizeof(wavHead);
    aparam->channels = head.channels;
    aparam->samplerate = head.samplerate;
    aparam->bitrate = head.byterate * 8;
    duration = dataSize / (aparam->bitrate / 8);
    MLOGI("This audio format is WAV. channels: %d, samplerate：%d",aparam->channels,aparam->samplerate);
    fseek(fp, sizeof(wavHead), SEEK_SET);
    return true;
}

void* wavdecoder::decode(int* size)
{
    int ret = fread(rbuf, sizeof(char), BUFFER_SIZE, fp);
    if(ret > 0) {
        fpos += ret;
        *size = ret;
    }
    return rbuf;
}

int wavdecoder::seek(int sec)
{
    float percent = (float)sec / duration;
    fpos = percent * dataSize;
    fseek(fp, fpos+sizeof(wavHead), SEEK_SET);
    return 0;
}

void wavdecoder::stop()
{
    if(mdecoder != NULL)
        delete mdecoder;
    mdecoder = NULL;
}

int wavdecoder::getDuration()
{
    MLOGI("Audio Duration %ds",duration);
    return duration;
}

wavdecoder* wavdecoder::mdecoder = NULL;
wavdecoder* wavdecoder::getInstance(const char* fname, audioParams *aparam)
{
    if(mdecoder == NULL)
        mdecoder = new wavdecoder();
    if(!mdecoder->Init(fname, aparam)) {
        MLOGE("Init Failed");
        delete mdecoder;
        mdecoder = NULL;
    }
    return mdecoder;
}
