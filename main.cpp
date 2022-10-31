#include <stdio.h>
#include "mlog.h"
#include "alsaOut.h"
#include "player.h"

int main(int argc, char *argv[])
{
    if(argc < 2) {
        printf("Usage: ./player filename\n");
        return -1;
    } 
    audioplayer *player = new audioplayer();
    player->Init(argv[1]);
    player->getDuration();
    player->start();
    sleep(5);
    player->seek(0);
    sleep(5);
    player->pause();
    player->stop();
    player->seek(227);
    player->seek(0);
    sleep(5);
    player->start();
    player->seek(45);
    sleep(5);
    player->stop();

    return 0;
}
