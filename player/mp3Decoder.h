#ifndef _MP3DECODER_H_
#define _MP3DECODER_H_

#include "decoderImp.h"
#include "mad.h"

typedef struct {
    char header[3];
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    char genre;
} MP3ID3V1;

class mp3decoder : public decoderImp
{
    public:
        virtual bool Init(const char *fname, audioParams *aparam);
        virtual void* decode(int* size);
        virtual int  seek(int sec);
        virtual void stop();
        virtual int getDuration();
        static mp3decoder* getInstance(const char *fname, audioParams* aparam);

    private:
        mp3decoder();
        ~mp3decoder();
        static mp3decoder* mdecoder;

        struct buffer {
            FILE *mfp;
            unsigned int fbsize;
            unsigned int fpos;
            unsigned char *sbuf;        //read from file
            unsigned char *dbuf;        //write for alsa
            struct mad_decoder decoder;
            audioParams *aparam;
            bool mstop;
        };
        struct buffer mbuf;

        int head_parser(FILE *fp, unsigned long *len, unsigned int *label_size);
        //preprocess  callback function
        static enum mad_flow input_preprocess(void *data, struct mad_stream *stream);
        static enum mad_flow header_preprocess(void *data,  struct mad_header const *header );
        //decode callback function
        static enum mad_flow header(void *data,  struct mad_header const *header );
        static enum mad_flow input(void *data, struct mad_stream *stream);
        static enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm);
        static enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame);

};


#endif /* _MP3DECODER_H_ */

