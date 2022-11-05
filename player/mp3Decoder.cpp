#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include "mp3Decoder.h"
#include "mlog.h"

/* global variable （Fix Info）*/
static unsigned long file_size;         //audio file size
static unsigned int label_size;         //audio label size
static unsigned int frame_size;         //frame size
static unsigned int duration;
static unsigned int current_sec;        //playback progress(second)
static unsigned int ibufsize;
static unsigned int obufsize;
static bool decodeLoop_isdone;
MP3ID3V1 minfo;

/*  read - write sync control */
std::mutex glock;
std::condition_variable mCond;
static bool isfull;

mp3decoder::mp3decoder()
{
    file_size = 0; 
    label_size = 0;
    frame_size = 0;
    duration = 0;
    current_sec = 0;
    ibufsize = 0;
    obufsize = 0;
    decodeLoop_isdone = false;
    memset(&mbuf, 0, sizeof(buffer));
    memset(&minfo, 0, sizeof(MP3ID3V1));
}

mp3decoder::~mp3decoder()
{
    free(mbuf.sbuf);
    free(mbuf.dbuf);
    fclose(mbuf.mfp);
    mbuf.mfp = NULL;
}

void mp3decoder::decodeLoop()
{
    mad_decoder_init(&mbuf.decoder, &mbuf,
            input, 0 /* header */, 0/*filter*/, output,
            error, 0/*message*/);
    mad_decoder_run(&mbuf.decoder, MAD_DECODER_MODE_SYNC);
    mad_decoder_finish(&mbuf.decoder);
    decodeLoop_isdone = true;
}

bool mp3decoder::Init(const char* fname, audioParams *aparam)
{
    mbuf.mfp = fopen(fname, "rb");
    if(mbuf.mfp == NULL){
        MLOGE("Failed open %s!", fname);
        return false;
    }
    mbuf.aparam = aparam;
    /* preprocess decode to get param */
    mad_decoder_init(&mbuf.decoder, &mbuf,
            input_preprocess, header_preprocess /* header */, 0 /* filter */, 0,
            error/*error*/,  0 /* message */);
    mad_decoder_run(&mbuf.decoder, MAD_DECODER_MODE_SYNC);
    mad_decoder_finish(&mbuf.decoder);

    mbuf.sbuf = (unsigned char *)malloc(ibufsize);
    mbuf.dbuf = (unsigned char *)malloc(obufsize);

    /* decode process */
    fseek(mbuf.mfp, label_size, SEEK_SET);
    decodeThread = std::thread(&mp3decoder::decodeLoop, this);
    decodeThread.detach();
    return true;
}

void* mp3decoder::decode(int* size)
{
    std::unique_lock<std::mutex> lck(glock);
    if(feof(mbuf.mfp)) {
        *size = -1;
        MLOGI("File play finish!");
        return (void *)-1;
    }
    isfull = false;
    mCond.notify_one();
    mCond.wait(lck, []{return isfull;});
    *size = obufsize;
    return mbuf.dbuf;
}

int mp3decoder::seek(int sec)
{
    std::unique_lock<std::mutex> lck(glock);
    mbuf.fpos = sec * mbuf.aparam->bitrate / 8;
    int seek_size = mbuf.fpos + label_size;
    fseek(mbuf.mfp, seek_size, SEEK_SET);
    return 0;
}

void mp3decoder::stop()
{
    mbuf.mstop = true;
    isfull = false;
    mCond.notify_one();
    while(!decodeLoop_isdone)
        usleep(100);
    if(mdecoder != NULL)
        delete mdecoder;
    mdecoder = NULL;
}

int mp3decoder::getDuration()
{
    std::unique_lock<std::mutex> lck(glock);
    MLOGI("Audio Duration %ds",duration);
    return duration;
}

enum mad_flow mp3decoder::input(void *data, struct mad_stream *stream)
{
    std::unique_lock<std::mutex> lck(glock);
    mCond.wait(lck, []{return !isfull;});
    int data_size = 0, copy_size = 0;
    int dataLen = file_size - label_size;
    struct buffer *buf = (struct buffer*)data;

    if(buf->fpos < dataLen) {
        data_size = stream->bufend - stream->next_frame;
        memcpy(buf->sbuf, buf->sbuf + buf->fbsize - data_size, data_size);
        copy_size = ibufsize - data_size;

        if(buf->fpos + copy_size > dataLen) {
            copy_size = dataLen - buf->fpos;
            fseek(buf->mfp, 0, SEEK_END);
        }
        fread(buf->sbuf + data_size, 1, copy_size, buf->mfp);
        buf->fpos += copy_size;
        buf->fbsize =  copy_size + data_size;

        mad_stream_buffer(stream, buf->sbuf, buf->fbsize);
    } 
    return MAD_FLOW_CONTINUE;
}

static inline signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L << (MAD_F_FRACBITS - 16));

    /* clip */
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    /* quantize */
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

enum mad_flow mp3decoder::output(void *data, struct mad_header const *header, struct mad_pcm *pcm)
{
    struct buffer *buf = (struct buffer*)data;
    unsigned int nchannels = 0, nsamples = 0;
    mad_fixed_t const *left_ch=NULL, *right_ch=NULL;

    /* pcm->samplerate contains the sampling frequency */
    nchannels = pcm->channels;
    nsamples  = pcm->length;
    left_ch   = pcm->samples[0];
    right_ch  = pcm->samples[1];

    int i = 0;
    while (nsamples--) {
        signed int sample = 0;;
        /* output sample in 16-bit signed little-endian PCM */
        sample = scale(*left_ch++);
        buf->dbuf[i++] = (sample >> 0) & 0xff;
        buf->dbuf[i++] = (sample >> 8) & 0xff;
        if (nchannels == 2) {
            sample = scale(*right_ch++);
            buf->dbuf[i++] = (sample >> 0) & 0xff;
            buf->dbuf[i++] = (sample >> 8) & 0xff;
        }
    }
    isfull = true;
    mCond.notify_one();

    if(buf->mstop)
        return MAD_FLOW_STOP;
    return MAD_FLOW_CONTINUE;
}

enum mad_flow mp3decoder::error(void *data, struct mad_stream *stream, struct mad_frame *frame)
{
    struct buffer *buf = (struct buffer*)data;
    MLOGE("decoding error 0x%04x (%s) at byte offset %lu",\
            stream->error, mad_stream_errorstr(stream),\
            stream->this_frame - buf->sbuf);
    /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */
    return MAD_FLOW_CONTINUE;
}

 /* preprocess callback function, pre_process parser params */
enum mad_flow mp3decoder::input_preprocess(void *data, struct mad_stream *stream)
{
    struct buffer *buf = (struct buffer *)data;
    unsigned char temp_buf[PREPROCESS_BUFSIZE] = {0};

    /* parser ID3V1 header, last 128 bytes */
    fseek(buf->mfp, -128, SEEK_END);
    fread(&minfo, sizeof(char), 128, buf->mfp);
    if(strncmp(minfo.header, "TAG", 3)!= 0) {
        MLOGE("NO ID3V1 TAG");
    }
    MLOGI("<< %s >> is play by '%s'", minfo.title, minfo.artist);

    /* parser ID3V2 header, first 10 bytes */
    char label[10];
    fseek(buf->mfp, 0, SEEK_SET);
    fread(label, 1, sizeof(label), buf->mfp);
    unsigned long fsta = ftell(buf->mfp);
    fseek(buf->mfp, 0, SEEK_END);
    unsigned long fend = ftell(buf->mfp);
    file_size = fend - fsta;
    label_size = (((label[6]&0x7F)<<21) | ((label[7]&0x7F)<<14) | ((label[8]&0x7F)<<7) | ((label[9]&0x7F))) + 10;
    MLOGI("File length %ld, label length %d", file_size, label_size);

    /* read PREPROCESS_BUFSIZE byte for params parser */
    fseek(buf->mfp, label_size, SEEK_SET);
    fread(temp_buf, sizeof(char), PREPROCESS_BUFSIZE, buf->mfp);
    mad_stream_buffer(stream, temp_buf, PREPROCESS_BUFSIZE);
    return MAD_FLOW_CONTINUE;
}

/* preprocess callback function, read frame head to get audio parameters */
 enum mad_flow mp3decoder::header_preprocess(void *data,  struct mad_header const *header)
{
    struct buffer *buf = (struct buffer *)data;

    /* get audio format info for alsa hw*/
    buf->aparam->bitrate = header->bitrate;
    buf->aparam->samplerate = header->samplerate;
    if(header->mode == 0)
        buf->aparam->channels = 1;
    else
        buf->aparam->channels = 2;
    MLOGI("samplerate: %d, channels: %d, bitrate: %ld",header->samplerate,buf->aparam->channels,header->bitrate);

    /*
     * set audio input output buffer size
     * 每帧长度 = ( 每帧采样数 / 8 * 比特率) / 采样频率 + 帧长调节
     * 播放时长 = ( 文件大小 –  ID3大小 ) × 8 ÷ 比特率(bit/s)
     */
    frame_size = (1152 / 8 * header->bitrate) / header->samplerate + (header->crc_check==1?2:0);
    if(header->bitrate % 32 == 0) {
        duration = (file_size - label_size) * 8 / header->bitrate;
        MLOGI("Check bitrate  mode: CBR, duration: %ds and frame_size is %d",duration, frame_size);
    }else
        MLOGW("Check bitrate mode : VBR. Need recalculate! [to do]");
    ibufsize = frame_size * 2;
    obufsize = 1152 * 2 * buf->aparam->channels;

    return MAD_FLOW_STOP;
}

mp3decoder* mp3decoder::mdecoder = NULL;
mp3decoder* mp3decoder::getInstance(const char* fname, audioParams *aparam)
{
    if(mdecoder == NULL)
        mdecoder = new mp3decoder();
    if(!mdecoder->Init(fname, aparam)) {
        MLOGE("Init Failed");
        delete mdecoder;
        mdecoder = NULL;
    }
    return mdecoder;
}
