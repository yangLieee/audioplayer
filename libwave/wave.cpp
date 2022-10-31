#include <stdio.h>
#include <string.h>
#include "wave.h"
#include "mlog.h"

#define MLOG_LEVEL DEBUG

void wave_create_header(wavHead *head, int size)
{
    strcpy(head->riff, "RIFF");
    head->size = size + 44;
    strcpy(head->wave, "WAVE");
    strcpy(head->fmt, "fmt");
    head->subsize = 16;
    head->audioformat = 1;
    head->channels = 2;
    head->samplerate = 48000;
    head->byterate = 64000;
    head->blookalign = 4;
    head->bits = 16;
    strcpy(head->subchunkid, "data");
    head->voicesize = size - 36;
}

void wave_get_header(FILE *wavFile, wavHead *head)
{
    if(wavFile == NULL)
        MLOGE("The file is NULL\n");
    fseek(wavFile, 0, SEEK_SET);
    fread(head, sizeof(struct wavHead), 1, wavFile);
}

int wave_get_fsize(FILE *wavFile)
{
    int fsize, fstat, fend;
    fstat = ftell(wavFile);
    fseek(wavFile, 0, SEEK_END);
    fend = ftell(wavFile);
    fsize = fend - fstat;
    return fsize;
}


