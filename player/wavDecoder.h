#ifndef _WAVDECODER_H_
#define _WAVDECODER_H_

#include "decoderImp.h"
#include "wave.h"

#define BUFFER_SIZE 4096

class wavdecoder : public decoderImp
{
    public:
        virtual bool Init(const char *fname, audioParams *aparam);
        virtual void* decode(int* size);
        virtual int  seek(int sec);
        virtual void stop();
        virtual int getDuration();
        static wavdecoder* getInstance(const char *fname, audioParams* aparam);

    private:
        wavdecoder();
        ~wavdecoder();

        FILE *fp;
        int dataSize;
        int fpos;
        int duration;
        struct wavHead head;
        unsigned char rbuf[BUFFER_SIZE];


        static wavdecoder* mdecoder;

};


#endif /* _WAVDECODER_H_ */

