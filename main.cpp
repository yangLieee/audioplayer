#include <stdio.h>
#include "mlog.h"
#include "alsaOut.h"

int main(int argc, char *argv[])
{
    alsaOut* alsa = new alsaOut();
    alsa->openDevice("default", 2, 48000);
    FILE *fp = fopen(argv[1], "rb");
    fseek(fp, SEEK_SET, 44);
    uint16_t buf[4096];
    while(1) {
        fread(buf, 1, 4096, fp);
        alsa->playback(buf, 1024);
    }
    return 0;
}
