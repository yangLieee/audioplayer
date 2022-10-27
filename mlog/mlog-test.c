#include <stdio.h>
#include "mlog.h"
#define MLOG_LEVEL INFO 

int main()
{
    MLOG();
    MLOGE("error");
    MLOGW("warning");
    MLOGI("info");
    MLOGD("debug");
    return 0;
}
