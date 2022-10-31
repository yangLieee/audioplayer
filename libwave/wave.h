#ifndef _WAVE_H_
#define _WAVE_H_

struct wavHead
{
    char riff[4];        //"RIFF"
    unsigned int size;          //wavFile size except riff
    char wave[4];        //"WAVE"
    char fmt[4];         //"FMT"
    unsigned int subsize;       //transition byte PCM=16
    unsigned short audioformat;   //PCM=1
    unsigned short channels;
    unsigned int samplerate;
    unsigned int byterate;      //channels*samplerates*16/8
    unsigned short blookalign;    //channels*16/8
    unsigned short bits;          //bits per samples
    char subchunkid[4];   //"DATA"
    unsigned short voicesize;
};

void wave_create_header(wavHead *head, int size);
void wave_get_header(FILE *wavFile, wavHead *head);
int wave_get_fsize(FILE *wavFile);

#endif /* _WAVE_H_ */

