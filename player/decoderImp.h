#ifndef _DECODERIMP_H_
#define _DECODERIMP_H_

struct audioParams
{
    char channels;
    int samplerate;
    int bitrate;
    // format
};

class decoderImp
{
    public:
        ~decoderImp() {
        }

        virtual bool Init(const char* fname, audioParams *aparam) = 0;
        virtual void*  decode(int* size) = 0;
        virtual int  seek(int sec) = 0;
        virtual void stop() = 0;
        virtual int getDuration() = 0;

};

#endif /* _DECODERIMP_H_ */

